#include "ServerImpl.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <thread>

#include <pthread.h>
#include <signal.h>
#include <chrono>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <afina/Storage.h>
#include <protocol/Parser.h>
#include <afina/execute/Command.h>

namespace Afina {
namespace Network {
namespace Blocking {

void *ServerImpl::RunAcceptorProxy(void *p) {
    ServerImpl *srv = reinterpret_cast<ServerImpl *>(p);
    try {
        srv->RunAcceptor();
    } catch (std::runtime_error &ex) {
        std::cerr << "Server fails: " << ex.what() << std::endl;
    }
    return 0;
}

// See Server.h
ServerImpl::ServerImpl(std::shared_ptr<Afina::Storage> ps) : Server(ps) {}

// See Server.h
ServerImpl::~ServerImpl() {}

// See Server.h
void ServerImpl::Start(uint32_t port, uint16_t n_workers) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    // If a client closes a connection, this will generally produce a SIGPIPE
    // signal that will kill the process. We want to ignore this signal, so send()
    // just returns -1 when this happens.
    sigset_t sig_mask;
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &sig_mask, NULL) != 0) {
        throw std::runtime_error("Unable to mask SIGPIPE");
    }

    // Setup server parameters BEFORE thread created, that will guarantee
    // variable value visibility
    max_workers = n_workers;
    listen_port = port;

    // The pthread_create function creates a new thread.
    //
    // The first parameter is a pointer to a pthread_t variable, which we can use
    // in the remainder of the program to manage this thread.
    //
    // The second parameter is used to specify the attributes of this new thread
    // (e.g., its stack size). We can leave it NULL here.
    //
    // The third parameter is the function this thread will run. This function *must*
    // have the following prototype:
    //    void *f(void *args);
    //
    // Note how the function expects a single parameter of type void*. We are using it to
    // pass this pointer in order to proxy call to the class member function. The fourth
    // parameter to pthread_create is used to specify this parameter value.
    //
    // The thread we are creating here is the "server thread", which will be
    // responsible for listening on port 23300 for incoming connections. This thread,
    // in turn, will spawn threads to service each incoming connection, allowing
    // multiple clients to connect simultaneously.
    // Note that, in this particular example, creating a "server thread" is redundant,
    // since there will only be one server thread, and the program's main thread (the
    // one running main()) could fulfill this purpose.
    running.store(true);
    if (pthread_create(&accept_thread, NULL, ServerImpl::RunAcceptorProxy, this) < 0) {
        throw std::runtime_error("Could not create server thread");
    }
}

// See Server.h
void ServerImpl::Stop() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    running.store(false);
}

// See Server.h
void ServerImpl::Join() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    pthread_join(accept_thread, 0);

    for (auto &thread : connections) {
        std::cout << "wait for " << thread.get_id() << std::endl;
        thread.join();
    }
}

// See Server.h
void ServerImpl::RunAcceptor() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    // For IPv4 we use struct sockaddr_in:
    // struct sockaddr_in {
    //     short int          sin_family;  // Address family, AF_INET
    //     unsigned short int sin_port;    // Port number
    //     struct in_addr     sin_addr;    // Internet address
    //     unsigned char      sin_zero[8]; // Same size as struct sockaddr
    // };
    //
    // Note we need to convert the port to network order

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;          // IPv4
    server_addr.sin_port = htons(listen_port); // TCP port number
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any address

    // Arguments are:
    // - Family: IPv4
    // - Type: Full-duplex stream (reliable)
    // - Protocol: TCP
    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        throw std::runtime_error("Failed to open socket");
    }

    // when the server closes the socket,the connection must stay in the TIME_WAIT state to
    // make sure the client received the acknowledgement that the connection has been terminated.
    // During this time, this port is unavailable to other processes, unless we specify this option
    //
    // This option let kernel knows that we are OK that multiple threads/processes are listen on the
    // same port. In a such case kernel will balance input traffic between all listeners (except those who
    // are closed already)
    int opts = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts)) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket setsockopt() failed");
    }

    // Bind the socket to the address. In other words let kernel know data for what address we'd
    // like to see in the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket bind() failed");
    }

    // Start listening. The second parameter is the "backlog", or the maximum number of
    // connections that we'll allow to queue up. Note that listen() doesn't block until
    // incoming connections arrive. It just makes the OS aware that this process is willing
    // to accept connections on this socket (which is bound to a specific IP and port)
    if (listen(server_socket, 5) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket listen() failed");
    }

    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t sinSize = sizeof(struct sockaddr_in);

    fd_set rfds;
    struct timeval tv;


    int select_retval;
    while (running.load()) {

        tv.tv_sec = 0;
        tv.tv_usec = 1000000;
        FD_ZERO(&rfds);
        FD_SET(server_socket, &rfds);
        select_retval = select(server_socket + 1, &rfds, NULL, NULL, &tv);
        if (select_retval == -1) {
            std::cerr << "[main] Error while select" << std::endl;
            break;
        } else if (!select_retval) {
            std::cerr << "[main] No data for 1000ms" << std::endl;
            continue;
        }

        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sinSize)) == -1) {
            close(server_socket);
            throw std::runtime_error("Socket accept() failed");
        }
        std::cout << "[main] going to add new thread" << std::endl;
        if (connections.size() == max_workers) {
            for (auto it = connections.begin(); it != connections.end();) {
                if (!threads_status.is_alive(it->get_id())) {
                    std::cout << "join thread: " << it->get_id() << std::endl;
                    it->join();
                    connections.erase(it);
                } else {
                    ++it;
                }
            }
        }

        std::cout << "[main]" << " current workers num = " << connections.size() << std::endl;
        if (connections.size() == max_workers) {
            std::cout << "Number of workers is too big, close connection :(" << std::endl;
            close(client_socket);
        } else {
            try {
                connections.push_back(std::thread(&ServerImpl::Worker, this, client_socket));
                threads_status.add_thread(connections.back().get_id());
            } catch (std::runtime_error &ex) {
                std::cerr << ex.what() << std::endl;
            }
        }
    }
    close(server_socket);
}

// See ServerImpl.h
void ServerImpl::Worker(int client_socket) {
    std::string data;

    bool client_connected = true;

    Socket client(client_socket);

    while (client_connected && running.load()) {
        data.clear();
        std::cout << "[" << std::this_thread::get_id() << "]" << "wait read" << std::endl;

        // Read data from client socket
        client.Read(data);

        // Check if client is still connected
        client_connected = !client.is_closed();
        if (!client_connected) {
            std::cout << "[" << std::this_thread::get_id() << "]" <<  "cliend disconnected" << std::endl;
            continue;
        }

        // Check if no errors happened
        if (!client.good()) {
            std::cout << "[" << std::this_thread::get_id() << "]" << "Error while read happend" << std::endl;
            client_connected = false;
            continue;
        }

        Protocol::Parser parser;

        size_t parsed = 0;
        parser.Parse(data, parsed);

        uint32_t body_size;
        auto command = parser.Build(body_size);
        std::string body = std::string(data.begin() + parsed,
                                       data.begin() + parsed + body_size);

        std::string out;
        command->Execute(*pStorage, body, out);

        std::cout << "[" << std::this_thread::get_id() << "]" << "send" << std::endl;
        send(client_socket, out.data(), out.size(), 0);
    }
    threads_status.add_done(std::this_thread::get_id());
}


void ThreadsStatus::add_thread(std::thread::id thread_id) {
    std::lock_guard<std::mutex> lock(_lock);

    _thread_alive[thread_id] = true;
}

bool ThreadsStatus::is_alive(std::thread::id thread_id) const {
    std::lock_guard<std::mutex> lock(_lock);

    auto it = _thread_alive.find(thread_id);
    if (it == _thread_alive.end())
        return false;

    return it->second;
}

void ThreadsStatus::add_done(std::thread::id thread_id) {
    std::lock_guard<std::mutex> lock(_lock);

    auto it = _thread_alive.find(thread_id);
    if (it == _thread_alive.end())
        return;

    it->second = false;
}

Socket::Socket(int fh) : _fh(fh), _good(true), _closed(false) {}

Socket::~Socket() {
    close(_fh);
}

void Socket::Read(std::string &out) {

    char buffer[32];
    ssize_t has_read = 0;

    has_read = read(_fh, buffer, 32);
    std::cout << "[" << std::this_thread::get_id() << "]" << "has_read_blockig = " << has_read << std::endl;
    if (!has_read) {
        _closed = true;
        return;
    }

    out += std::string(buffer, buffer + has_read);
    if (!this->_make_non_blocking()) {
        _good = false;
        return;
    }

    while ((has_read = read(_fh, buffer, 32)) > 0) {
        std::cout << "[" << std::this_thread::get_id() << "]" << "has_read = " << has_read << std::endl;
        out += std::string(buffer, buffer + has_read);
    }

    std::cout << "[" << std::this_thread::get_id() << "]" << "head_read_after = " << has_read << ", errno = " << errno << std::endl;

    if (has_read < 0 && errno == EAGAIN) {
        std::cout << "[" << std::this_thread::get_id() << "]" <<  "The socket is empty, make it blocking back" << std::endl;
        if (!this->_male_blokcing()) {
            _good = false;
            return;
        }
    } else if (has_read == 0 && errno == 0) {
        std::cout << "[" << std::this_thread::get_id() << "]" <<  "Client closed connection" << std::endl;
        _closed = true;
    }
}

bool Socket::good() const {
    return _good;
}

bool Socket::is_closed() const {
    return _closed;
}

bool Socket::_make_non_blocking() {
    int flags = fcntl(_fh, F_GETFL, 0);
    if (fcntl(_fh, F_SETFL, flags | O_NONBLOCK)) {
        std::cerr << "Can not change flags of file handler = " << _fh << std::endl;
        return false;
    }

    return true;
}

bool Socket::_male_blokcing() {
    int flags = fcntl(_fh, F_GETFL, 0);
    if (fcntl(_fh, F_SETFL, flags & ~O_NONBLOCK)) {
        std::cerr << "Can not change flags of file handler = " << _fh << std::endl;
        return false;
    }

    return true;
}
} // namespace Blocking
} // namespace Network
} // namespace Afina
