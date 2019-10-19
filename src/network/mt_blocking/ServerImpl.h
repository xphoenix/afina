#ifndef AFINA_NETWORK_MT_BLOCKING_SERVER_H
#define AFINA_NETWORK_MT_BLOCKING_SERVER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_set>

#include <afina/network/Server.h>

namespace spdlog {
class logger;
}

namespace Afina {
namespace Network {
namespace MTblocking {

/**
 * # Network resource manager implementation
 * Server that is spawning a separate thread for each connection
 */
class ServerImpl : public Server {
public:
    ServerImpl(std::shared_ptr<Afina::Storage> ps, std::shared_ptr<Logging::Service> pl);
    ~ServerImpl();

    // See Server.h
    void Start(uint16_t port, uint32_t, uint32_t) override;

    // See Server.h
    void Stop() override;

    // See Server.h
    void Join() override;

protected:
    /**
     * Method is running in the connection acceptor thread
     */
    void OnRun();

    // Process new connection:
    // - read commands until socket alive
    // - execute each command
    // - send response
    void ProcessConnection(int client_socket);

private:
    // Logger instance
    std::shared_ptr<spdlog::logger> _logger;

    // Atomic flag to notify threads when it is time to stop. Note that
    // flag must be atomic in order to safely publisj changes cross thread
    // bounds
    std::atomic<bool> running;

    // Server socket to accept connections on
    int _server_socket;

    // Client sockets
    std::unordered_set<int> _cl_sockets;

    // Thread to run network on
    std::thread _thread;

    // Maximum number of connections
    uint32_t _max_accept;

    // Mutex for queue with closed connections threads' id
    std::mutex _worker_mutex;

    // Indicate that all workers are closed
    std::condition_variable _cv;

    // Indicate that workers can be joined
    std::condition_variable _cv_join;

    // Indicate that all existing connections are stopped
    bool _connections_stopped;
};

} // namespace MTblocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_MT_BLOCKING_SERVER_H
