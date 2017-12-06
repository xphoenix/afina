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
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <afina/Storage.h>

#include "Utils.h"
#include "Worker.h"

namespace Afina {
namespace Network {
namespace NonBlocking {

// See Server.h
ServerImpl::ServerImpl(std::shared_ptr<Afina::Storage> ps) : Server(ps) {}

// See Server.h
ServerImpl::~ServerImpl() {}


template <void (ServerImpl::*method)()> static void *PthreadProxy(void *p) {
    
    /*

    Э
    Позволяет преобразовывать любой указатель в указатель любого другого типа. 
    Также позволяет преобразовывать любой целочисленный тип в любой тип указателя и наоборот.
    */

    //it called by client and 
            ServerImpl *srv = reinterpret_cast<ServerImpl *>(p);
            try {
                (srv->*method)();
                return (void *)0;
            } catch (std::runtime_error &ex) {
                std::cerr << "Exception caught: " << ex.what() << std::endl;
                return (void *)-1;
            }
            // different return values may be used to motify whoever calls pthread_join
            // of error conditions
        }


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

    // Create server socket
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;         // IPv4
    server_addr.sin_port = htons(port);       // TCP port number
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any address

    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        throw std::runtime_error("Failed to open socket");
    }

    int opts = 1;
    if (setsockopt(server_socket, SOL_SOCKET, 0, &opts, sizeof(opts)) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket setsockopt() failed");
    }

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket bind() failed");
    }

    make_socket_non_blocking(server_socket);
    if (listen(server_socket, 5) == -1) {
        close(server_socket);
        throw std::runtime_error("Socket listen() failed");
    }

    //МАГИЯ С СОКЕТОМ ЗАКАНИВАЕТСЯ


    workers.resize(n_workers);
    running.store(true);
    for (uint16_t i = 0; i < n_workers; i++)
        if (pthread_create(&workers[i], NULL, &PthreadProxy<&ServerImpl::RunEpoll>, this) < 0)
            throw std::runtime_error("Could not create epoll thread");


}

// See Server.h
void ServerImpl::Stop() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    for (auto &worker : workers) {
        worker.Stop();
    }
}

// See Server.h
void ServerImpl::Join() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    for (auto &worker : workers) {
        worker.Join();
    }
}

static const size_t buffer_size = 16;

struct client_fd: ep_fd(){
    int epoll_fd;//номер потока
    std::shared_ptr<Afina::Storage> ps;//указатель на стораж
    std::vector<char> buf;//буфер
    size_t offset;//для буфера
    std::list<client_fd> &list;
    std::list<client_fd>::iterator self;
    bool bailout;//?
    Protocol::Parser parser;



    //init client
    client_fd(int fd_, int epoll_fd_, std::shared_ptr<Afina::Storage> ps_, std::list<client_fd> &list_,
                          std::list<client_fd>::iterator self_)
                        : ep_fd(fd_), epoll_fd(epoll_fd_), ps(ps_), offset(0), list(list_), self(self_), bailout(false) {
                    buf.resize(buffer_size);
                }



    void cleanup() {
        //удаляет отслужившие объекты
        epoll_modify(epoll_fd, EPOLL_CTL_DEL, 0, *this);
        shutdown(fd, SHUT_RDWR);
        close(fd);
        list.erase(self);
        // ISO C++ faq does allow even `delete this`, subject to it being done carefully
                }

    /*
    Кастую указатели к ep_fd*. 
    У ep_fd метод advance полностью виртуальный; 
    его необходимо переопределить.
    */
    void advance(uint32_t events) override {
        //POLLHUP | EPOLLERR - ошибки файлового дескриптора
        if (events & (EPOLLHUP | EPOLLERR))
            return cleanup();

    ssize_t len = 0; // return value of send() & recv()
    // try to exhaust whatever the client has sent us
    do {
        offset += len;
        if (buf.size() == offset)
            buf.resize(std::max(buf.size() * 2, buffer_size));
        len = recv(fd, buf.data() + offset, buf.size() - offset, 0);
     } while (len > 0);
     //if buffer is full mode then the 'send()' will fail with 'EAGAIN' or 'EWOULDBLOCK'
     if (errno != EWOULDBLOCK && errno != EAGAIN)
        return cleanup();
        if (!len)
            bailout = true;

// if we don't have any pending output, we can try parse new commands
    if (!out.size()) {
        try {
            for (;;) { // loop until we can't create more commands
                uint32_t body_size;
                // check if there's a pending command in the parser
                auto cmd = parser.Build(body_size);
                if (!cmd) { // maybe we can parse more and get it?
                    size_t parsed = 0;
                    parser.Parse(buf.data(), offset, parsed);
                    buf.erase(buf.begin(), buf.begin() + parsed);
                    offset -= parsed;
                    cmd = parser.Build(body_size);
                }
                if (cmd && (!body_size || offset >= body_size + 2)) {
                    // got a command and a body if required, will execute now
                    std::string body;
                    parser.Reset();
                    if (body_size) {
                        // supply the body
                        body.assign(buf.data(), body_size);
                        buf.erase(buf.begin(), buf.begin() + body_size + 2);
                        offset -= (body_size + 2);
                    }
                    std::string local_out;
                    cmd->Execute(*ps, body, local_out);
                    out += local_out;
                    out += std::string("\r\n");
                } else {
                    // we did our best, but we can't execute a command yet
                    // no chance of other commands until more data comes
                    break;
                }
            }
        } catch (std::runtime_error &e) {
            // if anything fails we just report the error to the user
            out += std::string("CLIENT_ERROR ") + e.what() + std::string("\r\n");
            bailout = true;
        }
    }

 // we have created pending output just now or have it from previous iteration
            if (out.size()) {
                do {
                    len = send(fd, out.data(), out.size(), 0);
                    if (len > 0)
                        out.erase(0, len);
                } while (out.size() && len > 0);
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                    return cleanup();
            }

            if (bailout)
                return cleanup();
        }
    };//end of client_fd






 void ServerImpl::RunEpoll() {
    //__PRETTY_FUNCTION__ contains the signature of the function as well as its bare name
     std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

      // first, create the epoll instance
    int epoll_sock = epoll_create1(0);


      // prepare the necessary objects to handle clients
    std::list<client_fd> client_list; //список клиентов
    listen_fd listening_object{server_socket, epoll_sock, pStorage, client_list};

    



 }

} // namespace NonBlocking
} // namespace Network
} // namespace Afina