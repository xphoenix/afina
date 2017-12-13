#ifndef AFINA_NETWORK_NONBLOCKING_SERVER_H
#define AFINA_NETWORK_NONBLOCKING_SERVER_H

#include "Utils.h"

#include <atomic>
#include <pthread.h>
#include <vector>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <pthread.h>
#include <signal.h>

#include <errno.h>
#include <sys/epoll.h>

#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#include <afina/Storage.h>
#include <afina/execute/Command.h>
#include <protocol/Parser.h>

#include <list>

#include <afina/network/Server.h>

namespace Afina {
namespace Network {
namespace NonBlocking {

// Forward declaration, see Worker.h
class Worker;

/**
 * # Network resource manager implementation
 * Epoll based server
 */
class ServerImpl : public Server {
public:
    ServerImpl(std::shared_ptr<Afina::Storage> ps);
    ~ServerImpl();

    // See Server.h
    void Start(uint32_t port, uint16_t workers) override;

    // See Server.h
    void Stop() override;

    // See Server.h
    void Join() override;
    friend void* Epoll_Proxy(void* p);

protected:
    /**
    Method is calling epoll_wait in a separate thread
    */
    void RunEpoll();

private:
    // Port to listen for new connections, permits access only from
    // inside of accept_thread
    // Read-only
    //uint32_t listen_port;

    // Thread that is accepting new connections
    //std::vector<Worker> workers;
    std::atomic<bool> running;

    // Thread accepting and crunching client connections
    std::vector<pthread_t> workers;

    // Port number to listen on
    uint16_t listen_port;


};

} // namespace NonBlocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_NONBLOCKING_SERVER_H
