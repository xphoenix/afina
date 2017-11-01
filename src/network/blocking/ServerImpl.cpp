#include "ServerImpl.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <pthread.h>
#include <signal.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <afina/Storage.h>
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
            
            void *ServerImpl::RunConnectionProxy(void *p) {
                ServerImpl *srv = reinterpret_cast<ServerImpl *>(p);
                try {
                    srv->RunConnection();
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
                // incoming connections arrive. It just makesthe OS aware that this process is willing
                // to accept connections on this socket (which is bound to a specific IP and port)
                if (listen(server_socket, 5) == -1) {
                    close(server_socket);
                    throw std::runtime_error("Socket listen() failed");
                }
                
                
                struct sockaddr_in client_addr;
                socklen_t sinSize = sizeof(struct sockaddr_in);
                std::unique_lock<std::mutex> lock(client_lock);
                while (running.load()) {
                    std::cout << "network debug: waiting for connection..." << std::endl;
                    
                    // When an incoming connection arrives, accept it. The call to accept() blocks until
                    // the incoming connection arrives
                    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sinSize)) == -1) {
                        close(server_socket);
                        throw std::runtime_error("Socket accept() failed");
                    }
                    // ждем пока сдохнут
                    for (auto i = connections.begin(); i != connections.end(); /*FIXME*/)
                    {
                        if(pthread_kill(*i, 0) != 0)
                        {
                            pthread_join(*i, nullptr);
                            
                            i = connections.erase(i);
                        }
                        else i++;
                    }
                    
                    if (connections.size()< max_workers)
                    {
                        pthread_t client_thread;
                        client_ok = false;
                        pthread_create(&client_thread, nullptr, RunConnectionProxy, this);
                        while (!client_ok) {
                            variable_lock.wait(lock);
                        }
                        connections.insert(client_thread);
                    }
                    else
                    {
                        shutdown(client_socket,SHUT_RDWR);
                        close(client_socket);
                        
                    }
                    //
                }
                
                for (auto i = connections.begin(); i != connections.end(); /*FIXME*/)
                {
                    
                    pthread_join(*i, nullptr);
                    i++;
                }
                
                // Cleanup on exit...
                shutdown(server_socket,SHUT_RDWR);
                close(server_socket);
                
                // Wait until for all connections to be complete
                std::unique_lock<std::mutex> __lock(connections_mutex);
                while (!connections.empty()) {
                    connections_cv.wait(__lock);
                }
            }
            
            
            static const size_t read_buffer_size = 256;
            
            
            // See Server.h
            void ServerImpl::RunConnection() {
                int client;
                {
                    std::lock_guard<std::mutex> lock(client_lock);
                    client = client_socket;
                    client_ok = true;
                    variable_lock.notify_one();
                }
                
                Protocol::Parser parser;
                std::vector<char> buf;
                buf.resize(read_buffer_size);
                size_t parsed = 0;
                size_t offset = 0;
                
                for (;;) {
                    std::string out;
                    bool bail_out = false;
                    // both parser and command may throw exceptions
                    try {
                        ssize_t received = 0;
                        // parse first because we may have saved this from the first iteration
                        while (!parser.Parse(buf.data() + parsed, offset - parsed, parsed)) {
                            // move excess data to the beginning of the buffer
                            memmove(buf.data(), buf.data() + parsed, offset - parsed);
                            offset -= parsed;
                            parsed = 0;
                            // append whatever the client may have sent
                            received = recv(client, buf.data() + offset, buf.size() - offset, 0);
                            if (received <= 0) { // client bails out, no command to execute
                                shutdown(client, SHUT_RDWR);
                                close(client);
                                return;
                            } else
                                offset += received;
                        }
                        
                        // parser.Parse returned true -- can build a command now
                        uint32_t arg_size;
                        auto cmd = parser.Build(arg_size);
                        parser.Reset(); // don't forget
                        
                        std::string arg;
                        // was there an argument?
                        if (arg_size) {
                            arg_size += 2; // data is followed by \r\n
                            if (arg_size > buf.size())
                                buf.resize(arg_size);
                            
                            if (offset - parsed) { // we've read body with the command
                                // move everything we have to the beginning
                                memmove(buf.data(), buf.data() + parsed, offset - parsed);
                                offset -= parsed; // and account for it
                            }
                            
                            while (offset < arg_size) { // append the body we know the size of
                                received = recv(client, buf.data() + offset, buf.size() - offset, 0);
                                if (received <= 0) { // client bails out, no data to store
                                    shutdown(client, SHUT_RDWR);
                                    close(client);
                                    return;
                                }
                                offset += received;
                            }
                            // prepare the body
                            arg.assign(buf.data(), arg_size - 2 /* account for extra \r\n */);
                            // move the remainings of the buffer to the beginning and fix offsets & sizes
                            memmove(buf.data(), buf.data() + arg_size, offset - arg_size);
                            parsed = 0;
                            offset -= arg_size;
                        }
                        
                        // time to do the deed
                        cmd->Execute(*pStorage, arg, out); // ловим исклчюение и выдаем его ниже
                    } catch (std::runtime_error &e) {
                        // if anything fails we just report the error to the user
                        out = std::string("CLIENT_ERROR ") + e.what();
                        bail_out = true;
                    }
                    
                    if (out.size()) {
                        out += std::string("\r\n");
                        size_t offset = 0;
                        ssize_t sent;
                        while (offset < out.size()) { // classical "send until nothing left or error" loop
                            sent = send(client, out.data() + offset, out.size() - offset, 0);
                            if (sent <= 0) { // client bails out, reply not sent
                                shutdown(client, SHUT_RDWR);
                                close(client);
                                return;
                            }
                            offset += sent;
                        }
                        if (bail_out) {
                            shutdown(client, SHUT_RDWR);
                            close(client);
                            return;
                        }
                    }
                    
                }
            }
        } // namespace Blocking
    } // namespace Network
} // namespace Afina
