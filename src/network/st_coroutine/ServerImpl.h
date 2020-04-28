#ifndef AFINA_NETWORK_ST_COROUTINE_SERVER_H
#define AFINA_NETWORK_ST_COROUTINE_SERVER_H

#include <thread>
#include <vector>
#include <arpa/inet.h>

#include <afina/network/Server.h>
#include <afina/coroutine/Engine.h>
#include "Connection.h"

namespace spdlog {
class logger;
}

namespace Afina {
namespace Network {
namespace STcoroutine {

/**
 * # Network resource manager implementation
 * Epoll based server
 */
class ServerImpl : public Server {
public:
    ServerImpl(std::shared_ptr<Afina::Storage> ps, std::shared_ptr<Logging::Service> pl);
    ~ServerImpl();

    // See Server.h
    void Start(uint16_t port, uint32_t acceptors, uint32_t workers) override;

    // See Server.h
    void Stop() override;

    // See Server.h
    void Join() override;

protected:
    void OnRun();

    void Worker(int fd);

private:
    // logger to use
    std::shared_ptr<spdlog::logger> _logger;

    // Port to listen for new connections, permits access only from
    // inside of accept_thread
    // Read-only
    uint16_t listen_port;

    // Socket to accept new connection on, shared between acceptors
    int _server_socket;

    // Curstom event "device" used to wakeup workers
    int _event_fd;

    int _epoll_descr;

    // IO thread
    std::thread _work_thread;

    Afina::Coroutine::Engine _engine;

    Connection *connections;

    bool _running;

    void unblocker();

    ssize_t _read(int fd, void *buf, size_t count, Connection *pc);

    ssize_t _write(int fd, const void *buf, size_t count, Connection *pc);

    int _accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, Connection *pc);
};

} // namespace STcoroutine
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_COROUTINE_SERVER_H
