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

#include "Connection.h"
#include "Utils.h"
#include "Worker.h"

namespace Afina {
namespace Network {
namespace MTnonblock {

// See Server.h
ServerImpl::ServerImpl(std::shared_ptr<Afina::Storage> ps, std::shared_ptr<Logging::Service> pl) : Server(ps, pl) {}

// See Server.h
ServerImpl::~ServerImpl() {}

// See Server.h
void ServerImpl::Start(uint16_t port, uint32_t n_acceptors, uint32_t n_workers) {
    _logger = pLogging->select("network");
    _logger->info("Start network service");

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

    // Start IO workers
    _data_epoll_fd = epoll_create1(0);
    if (_data_epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor: " + std::string(strerror(errno)));
    }

    _event_fd = eventfd(0, EFD_NONBLOCK);
    if (_event_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor: " + std::string(strerror(errno)));
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = nullptr;
    if (epoll_ctl(_data_epoll_fd, EPOLL_CTL_ADD, _event_fd, &event)) {
        throw std::runtime_error("Failed to add eventfd descriptor to epoll");
    }

    _workers.reserve(n_workers);
    for (int i = 0; i < n_workers; i++) {
        _workers.emplace_back(pStorage, pLogging);
        _workers.back().Start(_data_epoll_fd);
    }

    // Start acceptors
    _acceptors.reserve(n_acceptors);
    for (int i = 0; i < n_acceptors; i++) {
        _acceptors.emplace_back(&ServerImpl::OnRun, this);
    }
}

// See Server.h
void ServerImpl::Stop() {
    _logger->warn("Stop network service");
    // Said workers to stop
    for (auto &w : _workers) {
        w.Stop();
    }

    // Wakeup threads that are sleep on epoll_wait
    if (eventfd_write(_event_fd, 1)) {
        throw std::runtime_error("Failed to wakeup workers");
    }
}

// See Server.h
void ServerImpl::Join() {
    for (auto &t : _acceptors) {
        t.join();
    }

    for (auto &w : _workers) {
        w.Join();
    }
}

// See ServerImpl.h
void ServerImpl::OnRun() {
    _logger->info("Start acceptor");
    int acceptor_epoll = epoll_create1(0);
    if (acceptor_epoll == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor: " + std::string(strerror(errno)));
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLEXCLUSIVE;
    event.data.fd = _server_socket;
    if (epoll_ctl(acceptor_epoll, EPOLL_CTL_ADD, _server_socket, &event)) {
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }

    struct epoll_event event2;
    event2.events = EPOLLIN;
    event2.data.fd = _event_fd;
    if (epoll_ctl(acceptor_epoll, EPOLL_CTL_ADD, _event_fd, &event2)) {
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }

    bool run = true;
    std::array<struct epoll_event, 64> mod_list;
    while (run) {
        int nmod = epoll_wait(acceptor_epoll, &mod_list[0], mod_list.size(), -1);
        _logger->debug("Acceptor wokeup: {} events", nmod);

        for (int i = 0; i < nmod; i++) {
            struct epoll_event &current_event = mod_list[i];
            if (current_event.data.fd == _event_fd) {
                _logger->debug("Break acceptor due to stop signal");
                run = false;
                continue;
            }

            for (;;) {
                struct sockaddr in_addr;
                socklen_t in_len;

                // No need to make these sockets non blocking since accept4() takes care of it.
                in_len = sizeof in_addr;
                int infd = accept4(_server_socket, &in_addr, &in_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
                if (infd == -1) {
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                        break; // We have processed all incoming connections.
                    } else {
                        _logger->error("Failed to accept socket");
                        break;
                    }
                }

                // Print host and service info.
                char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
                int retval = getnameinfo(&in_addr, in_len, hbuf, sizeof hbuf, sbuf, sizeof sbuf,
                                         NI_NUMERICHOST | NI_NUMERICSERV);
                if (retval == 0) {
                    _logger->info("Accepted connection on descriptor {} (host={}, port={})\n", infd, hbuf, sbuf);
                }

                // Register the new FD to be monitored by epoll.
                Connection *pc = new Connection(infd);
                if (pc == nullptr) {
                    throw std::runtime_error("Failed to allocate connection");
                }

                // Register connection in worker's epoll
                pc->Start();
                if (pc->isAlive()) {
                    pc->_event.events |= EPOLLONESHOT;
                    if (epoll_ctl(_data_epoll_fd, EPOLL_CTL_MOD, pc->_socket, &pc->_event)) {
                        pc->OnError();
                        delete pc;
                    }
                }
            }
        }
    }
    _logger->warn("Acceptor stopped");
}

} // namespace MTnonblock
} // namespace Network
} // namespace Afina
