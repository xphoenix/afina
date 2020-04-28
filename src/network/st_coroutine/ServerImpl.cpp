#include "ServerImpl.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <spdlog/logger.h>

#include <afina/Storage.h>
#include <afina/logging/Service.h>
#include <protocol/Parser.h>

#include "Connection.h"
#include "Utils.h"

namespace Afina {
namespace Network {
namespace STcoroutine {

// See Server.h
ServerImpl::ServerImpl(std::shared_ptr<Afina::Storage> ps, std::shared_ptr<Logging::Service> pl) : Server(ps, pl),
    _engine([this]{this->unblocker();}), connections(nullptr), _running(false) {}

// See Server.h
ServerImpl::~ServerImpl() {}

// See Server.h
void ServerImpl::Start(uint16_t port, uint32_t n_acceptors, uint32_t n_workers) {
    _logger = pLogging->select("network");
    _logger->info("Start st_coroutine network service");

    sigset_t sig_mask;
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &sig_mask, NULL) != 0) {
        throw std::runtime_error("Unable to mask SIGPIPE");
    }

    // Create server socket
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;         // IPv4
    server_addr.sin_port = htons(port);       // TCP port number
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any address

    _server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_server_socket == -1) {
        throw std::runtime_error("Failed to open socket: " + std::string(strerror(errno)));
    }

    int opts = 1;
    if (setsockopt(_server_socket, SOL_SOCKET, (SO_KEEPALIVE), &opts, sizeof(opts)) == -1) {
        close(_server_socket);
        throw std::runtime_error("Socket setsockopt() failed: " + std::string(strerror(errno)));
    }

    if (bind(_server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(_server_socket);
        throw std::runtime_error("Socket bind() failed: " + std::string(strerror(errno)));
    }

    make_socket_non_blocking(_server_socket);
    if (listen(_server_socket, 5) == -1) {
        close(_server_socket);
        throw std::runtime_error("Socket listen() failed: " + std::string(strerror(errno)));
    }

    _event_fd = eventfd(0, EFD_NONBLOCK);
    if (_event_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor: " + std::string(strerror(errno)));
    }

    _epoll_descr = epoll_create1(0);
    if (_epoll_descr == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor: " + std::string(strerror(errno)));
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = _server_socket;
    if (epoll_ctl(_epoll_descr, EPOLL_CTL_ADD, _server_socket, &event)) {
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }

    struct epoll_event event2;
    event2.events = EPOLLIN;
    event2.data.fd = _event_fd;
    if (epoll_ctl(_epoll_descr, EPOLL_CTL_ADD, _event_fd, &event2)) {
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }

    _running = true;
    _work_thread = std::thread([this]{ this->_engine.start(static_cast<void(*)(ServerImpl *)>([](ServerImpl *s){ s->OnRun(); }), this); });
}

// See Server.h
void ServerImpl::Stop() {
    _logger->warn("Stop network service");

    for (auto pc = connections; pc != nullptr; pc = pc->_next) {
        pc->_running = false;
    }
    // Wakeup threads that are sleep on epoll_wait
    if (eventfd_write(_event_fd, 1)) {
        throw std::runtime_error("Failed to wakeup workers");
    }
    close(_server_socket);
}

// See Server.h
void ServerImpl::Join() {
    // Wait for work to be complete
    _work_thread.join();
}

// See ServerImpl.h
void ServerImpl::OnRun() {
    auto cur_rout = _engine.get_cur_routine();
    auto newconn = new Connection(_server_socket);
    newconn->_events = 0;
    newconn->_running = true;
    newconn->_ctx = cur_rout;
    connections = newconn;
    while (_running) {
        struct sockaddr in_addr;
        socklen_t in_len;
        in_len = sizeof(in_addr);
        int infd = _accept(_server_socket, &in_addr, &in_len, newconn);
        if (infd == -1) {
            continue;
        }

        // Print host and service info.
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        int retval =
                getnameinfo(&in_addr, in_len, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV);
        if (retval == 0) {
            _logger->info("Accepted connection on descriptor {} (host={}, port={})\n", infd, hbuf, sbuf);
        }

        _engine.run(static_cast<void(*)(ServerImpl *, int &)>([](ServerImpl *s, int &infd) { s->Worker(infd); }), this, infd);
    }
        if (newconn->_prev != nullptr) {
            newconn->_prev->_next = newconn->_next;
        }
        if (newconn->_next != nullptr) {
            newconn->_next->_prev = newconn->_prev;
        }
        if (connections == newconn) {
            connections = newconn->_next;
        }
        newconn->_prev = newconn->_next = nullptr;
        delete newconn;
}

void ServerImpl::Worker(int client_socket) {
    std::size_t arg_remains = 0;
    Protocol::Parser parser;
    std::string argument_for_command;
    std::unique_ptr<Execute::Command> command_to_execute;
    auto conn = new Connection(client_socket);
    conn->_events = 0;
    conn->_running = true;
    conn->_ctx = _engine.get_cur_routine();
    conn->_next = connections;
    connections = conn;
    if (conn->_next != nullptr) {
        conn->_next->_prev = conn;
    }
    try {
        int readed_bytes = -1;
        char client_buffer[4096] = "";
        while (_running && (readed_bytes = _read(client_socket, client_buffer, sizeof(client_buffer), conn)) > 0) {
            _logger->debug("Got {} bytes from socket", readed_bytes);

            while (readed_bytes > 0) {
                _logger->debug("Process {} bytes", readed_bytes);
                // There is no command yet
                if (!command_to_execute) {
                    std::size_t parsed = 0;
                    if (parser.Parse(client_buffer, readed_bytes, parsed)) {
                        // There is no command to be launched, continue to parse input stream
                        // Here we are, current chunk finished some command, process it
                        _logger->debug("Found new command: {} in {} bytes", parser.Name(), parsed);
                        command_to_execute = parser.Build(arg_remains);
                        if (arg_remains > 0) {
                            arg_remains += 2;
                        }
                    }

                    // Parsed might fails to consume any bytes from input stream. In real life that could happens,
                    // for example, because we are working with UTF-16 chars and only 1 byte left in stream
                    if (parsed == 0) {
                        break;
                    } else {
                        std::memmove(client_buffer, client_buffer + parsed, readed_bytes - parsed);
                        readed_bytes -= parsed;
                    }
                }

                // There is command, but we still wait for argument to arrive...
                if (command_to_execute && arg_remains > 0) {
                    _logger->debug("Fill argument: {} bytes of {}", readed_bytes, arg_remains);
                    // There is some parsed command, and now we are reading argument
                    std::size_t to_read = std::min(arg_remains, std::size_t(readed_bytes));
                    argument_for_command.append(client_buffer, to_read);

                    std::memmove(client_buffer, client_buffer + to_read, readed_bytes - to_read);
                    arg_remains -= to_read;
                    readed_bytes -= to_read;
                }
                // Thre is command & argument - RUN!
                if (command_to_execute && arg_remains == 0) {
                    _logger->debug("Start command execution");

                    std::string result;
                    command_to_execute->Execute(*pStorage, argument_for_command, result);
                    // Send response
                    result += "\r\n";
                    if (_write(client_socket, result.data(), result.size(), conn) == -1) {
                        break;
                    }

                    // Prepare for the next command
                    command_to_execute.reset();
                    argument_for_command.resize(0);
                    parser.Reset();
                }
            } // while (readed_bytes)
        }
        if (readed_bytes == 0) {
            _logger->debug("Connection closed");
        } else {
            throw std::runtime_error(std::string(strerror(errno)));
        }
    } catch (std::runtime_error &ex) {
        _logger->error("Failed to process connection on descriptor {}: {}", client_socket, ex.what());
    }
    if (conn->_prev != nullptr) {
        conn->_prev->_next = conn->_next;
    }
    if (conn->_next != nullptr) {
        conn->_next->_prev = conn->_prev;
    }
    if (connections == conn) {
        connections = conn->_next;
    }
    conn->_prev = conn->_next = nullptr;
    delete conn;
    close(client_socket);
}

void ServerImpl::unblocker() {
    std::array<struct epoll_event, 64> mod_list;
    while (_engine.is_all_blocked()) {
        int nmod = epoll_wait(_epoll_descr, &mod_list[0], mod_list.size(), -1);
        for (int i = 0; i < nmod; i++) {
            struct epoll_event &current_event = mod_list[i];
            if (current_event.data.fd == _event_fd) {
                _logger->debug("Break acceptor due to stop signal");
                _engine.unblock_all();
                continue;
            }
            Connection *pc = static_cast<Connection *>(current_event.data.ptr);
            Afina::Coroutine::Engine::context *ctx = pc->_ctx;
            _engine.unblock(ctx);
        }
    }
}

ssize_t ServerImpl::_read(int fd, void *buf, size_t count, Connection *pc) {
    while (pc->_running) {
        ssize_t bytes_read = read(fd, buf, count);
        if (bytes_read > 0) {
            return bytes_read;
        } else {
            struct epoll_event struct_events;
            struct_events.events =  EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
            struct_events.data.ptr = pc;
            epoll_ctl(_epoll_descr, EPOLL_CTL_ADD, fd, &struct_events));
            pc->_events = 0;
            _engine.block(nullptr);
            epoll_ctl(_epoll_descr, EPOLL_CTL_DEL, fd, &struct_events);

            if ((pc->_events & EPOLLRDHUP) || (pc->_events & EPOLLERR) || (pc->_events & EPOLLHUP)) {
                return read(fd, buf, count);
            }
        }
    }
    return -1;
}

ssize_t ServerImpl::_write(int fd, const void *buf, size_t count, Connection *pc) {
    ssize_t written = 0;
    while (pc->_running) {
        written += write(fd, static_cast<const void *>(static_cast<const char *>(buf) + written), count - written);
        if (written < count) {
            struct epoll_event struct_events;
            struct_events.events =  EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
            struct_events.data.ptr = pc;
            epoll_ctl(_epoll_descr, EPOLL_CTL_ADD, fd, &struct_events));
            pc->_events = 0;
            _engine.block(nullptr);
            epoll_ctl(_epoll_descr, EPOLL_CTL_DEL, fd, &struct_events);

            if ((pc->_events & EPOLLERR) || (pc->_events & EPOLLHUP)) {
                return -1;
            }
        } else {
            return written;
        }
    }
    return -1;
}

int ServerImpl::_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, Connection *pc) {
    while (pc->_running) {
        int fd = accept4(sockfd, addr, addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (fd == -1) {
            struct epoll_event struct_events;
            struct_events.events =  EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
            struct_events.data.ptr = pc;
            epoll_ctl(_epoll_descr, EPOLL_CTL_ADD, sockfd, &struct_events));
            pc->_events = 0;
            _engine.block(nullptr);
            epoll_ctl(_epoll_descr, EPOLL_CTL_DEL, sockfd, &struct_events);
        } else {
            return fd;
        }
    }
    return -1;
}

} // namespace STcoroutine
} // namespace Network
} // namespace Afina
