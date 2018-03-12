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

#include <sys/time.h>
#include <chrono>
#include <ctime>

namespace Afina {
namespace Network {
namespace Blocking {

std::mutex log_lock;
void log(std::string write, long long opt = -1123) {
    std::chrono::time_point<std::chrono::system_clock> now;
    now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);
    std::string cur_time = std::ctime(&end_time);
    cur_time.erase((cur_time.end() - 1));

    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    std::cout << "[" << std::this_thread::get_id() << ", " << ms << "]" <<  write;
    if (opt != -1123) {
        std::cout << ' ' << opt;
    }
    std::cout << std::endl;
}

void log(const char* str, long long opt = -1123) {
    std::lock_guard<std::mutex> lock(log_lock);
    int len = strlen(str);
    std::string tmp = std::string(str, str + len);
    log(tmp, opt);
}


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

    threads_status.join();
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
            continue;
        }

        log("update");
        threads_status.update();
        log("update done");
        bool close_immediately = (threads_status.size() == max_workers);
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sinSize)) == -1) {
            close(server_socket);
            throw std::runtime_error("Socket accept() failed");
        }
        if (close_immediately) {
            log("Number of workers is too big, close connection :(");
            close(client_socket);
            continue;
        }

        log("current workers num", threads_status.size());
        try {
            _dont_work.lock();
            threads_status.add_thread(std::thread(&ServerImpl::Worker, this, client_socket));
            _dont_work.unlock();
        } catch (std::runtime_error &ex) {
            std::cerr << ex.what() << std::endl;
        }

    }
    close(server_socket);
}

// See ServerImpl.h
void ServerImpl::Worker(int client_socket) {
    _dont_work.lock();
    _dont_work.unlock();
    log("in Worker");
    std::string data;

    bool client_connected = true;

    Socket client(client_socket);

    log("before loop");
    while (client_connected && running.load()) {
        data.clear();
        log("wait read");

        // Read data from client socket
        client.Read(data);

        // Check if client is still connected
        client_connected = !client.is_closed();
        if (!client_connected) {
            std::cout << "[" << std::this_thread::get_id() << "]" <<  "client disconnected" << std::endl;
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

void show(std::unordered_map<std::thread::id, bool>& map) {
    for (auto& it : map) {
        std::cout << "[main] [" << it.first << "] = " << it.second << std::endl;
    }
}

void ThreadsStatus::add_thread(std::thread thread) {
    std::lock_guard<std::mutex> lock(_lock);

    log("new thread id");
    auto it_map = statuses.find(thread.get_id());
    if (it_map != statuses.end()) {
        std::cout << "[main] race condition, try to find = " << thread.get_id();
        std::cout << " connections_size(" << connections.size() << ")" << std::endl;
        for (auto it = connections.begin(); it != connections.end();) {
            std::cout << "[main] look for: " << it->get_id() << std::endl;
            if (it->get_id() == thread.get_id()) {
                std::cout << "[main] join " << it->get_id() << std::endl;
                it->join();
                statuses.erase(it_map);
                connections.erase(it);
                break;
            } else {
                ++it;
            }
        }
    }

    statuses[thread.get_id()] = true;
    connections.push_back(std::move(thread));
    log("end of add thread");
}

// Only main, but access to common resource
bool ThreadsStatus::is_alive(std::thread::id thread_id) {
    std::lock_guard<std::mutex> lock(_lock);

    auto it = statuses.find(thread_id);
    if (it == statuses.end())
        return false;

    return it->second;
}

// Workers only
void ThreadsStatus::add_done(std::thread::id thread_id) {
    std::lock_guard<std::mutex> lock(_lock);

    log("add done");
    auto it = statuses.find(thread_id);
    if (it == statuses.end()) {
        log("wtf");
        return;
    }
    it->second = false;
}

size_t ThreadsStatus::size() const {
    return connections.size();
}

void ThreadsStatus::update() {
    std::lock_guard<std::mutex> lock(_lock);

    for (auto it = connections.begin(); it != connections.end();) {
        log("try to find");
        auto it_map = statuses.find(it->get_id());
        std::cout << "Thread " << it->get_id() << " in map = " << it_map->second << std::endl;
        if (it_map == statuses.end()) {
            std::cout << "[main] wtf " << it->get_id() << std::endl;
            connections.erase(it);
            ++it;
        } else if (!it_map->second) { // if thread is dead
            log("join");
            it->join();
            statuses.erase(it_map);
            connections.erase(it);
        } else {
            std::cout << "[main] thread " << it->get_id() << " is still running" << std::endl;
            ++it;
        }
    }
}

void ThreadsStatus::join() {
    for (auto& thread : connections) {
        std::cout << "[main] join " << thread.get_id() << std::endl;
        thread.join();
    }
}


Socket::Socket(int fh) : _fh(fh), _good(true), _closed(false) {}

Socket::~Socket() {
    close(_fh);
}

void Socket::Read(std::string &out) {

    char buffer[32];
    ssize_t has_read = 0;

    has_read = read(_fh, buffer, 32);
    std::cout << "[" << std::this_thread::get_id() << "]" << "has_read_blocking = " << has_read << std::endl;
    if (has_read <= 0) {
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
