#ifndef AFINA_NETWORK_MT_NONBLOCKING_WORKER_H
#define AFINA_NETWORK_MT_NONBLOCKING_WORKER_H

#include <atomic>
#include <memory>
#include <thread>

namespace spdlog {
class logger;
}

namespace Afina {

// Forward declaration, see afina/Storage.h
class Storage;
namespace Logging {
class Service;
}

namespace Network {
namespace MTnonblock {

/**
 * # Thread running epoll
 * On Start spaws background thread that is doing epoll on the given server
 * socket and process incoming connections and its data
 */
class Worker {
public:
    Worker(std::shared_ptr<Afina::Storage> ps, std::shared_ptr<Afina::Logging::Service> pl);
    ~Worker();

    Worker(Worker &&);
    Worker &operator=(Worker &&);

    /**
     * Spaws new background thread that is doing epoll on the given server
     * socket. Once connection accepted it must be registered and being processed
     * on this thread
     */
    void Start(int epoll_fd);

    /**
     * Signal background thread to stop. After that signal thread must stop to
     * accept new connections and must stop read new commands from existing. Once
     * all readed commands are executed and results are send back to client, thread
     * must stop
     */
    void Stop();

    /**
     * Blocks calling thread until background one for this worker is actually
     * been destoryed
     */
    void Join();

protected:
    /**
     * Method executing by background thread
     */
    void OnRun();

private:
    Worker(Worker &) = delete;
    Worker &operator=(Worker &) = delete;

    // afina services
    std::shared_ptr<Afina::Storage> _pStorage;

    // afina services
    std::shared_ptr<Afina::Logging::Service> _pLogging;

    // Logger to be used
    std::shared_ptr<spdlog::logger> _logger;

    // Flag signals that thread should continue to operate
    std::atomic<bool> isRunning;

    // Thread serving requests in this worker
    std::thread _thread;

    // EPOLL descriptor using for events processing
    int _epoll_fd;
};

} // namespace MTnonblock
} // namespace Network
} // namespace Afina
#endif // AFINA_NETWORK_MT_NONBLOCKING_WORKER_H
