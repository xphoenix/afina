#ifndef AFINA_NETWORK_BLOCKING_SERVER_H
#define AFINA_NETWORK_BLOCKING_SERVER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
//#include <logger/Logger.h>

#include <afina/network/Server.h>



namespace Afina {

namespace Network {
namespace Blocking {

// Class maganages threads' statutes
class ThreadsStatus {
public:
    ThreadsStatus() = default;
    ~ThreadsStatus() = default;

    // Add new thread to map
    void add_thread(std::thread);

    // Check is thread with id - thread_id is still running
    bool is_alive(std::thread::id);

    // Match thread width id - thread_id as done
    void add_done(std::thread::id);

    // Delete dead threads
    void update();

    // Join threads
    void join();

    size_t size() const;

private:
    std::mutex _lock;
    std::vector<std::thread> connections;

    // Thread + is_alive
    std::unordered_map<std::thread::id, bool> statuses;
};

// Class Socket for read all data from it
class Socket {
public:
    Socket(int fh);
    ~Socket();

    // Read all data from socket
    void Read(std::string& out);

    // Write all data to socket
    void Write(std::string& out);

    // Check if was an error
    bool good() const;

    // Check if socket closed
    bool is_closed() const;

private:
    int _fh;

    // Was error during operations
    bool _good;

    // Is fh closed
    bool _closed;

    // Makes fh non blocking
    bool _make_non_blocking();

    // Make fh blocking
    bool _male_blokcing();
};

/**
 * # Network resource manager implementation
 * Server that is spawning a separate thread for each connection
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

protected:
    /**
     * Method is running in the connection acceptor thread
     */
    void RunAcceptor();

    /**
     * Methos is running for each connection
     */
    void Worker(int, size_t);
private:
    static void* RunAcceptorProxy(void* p);

    // Atomic flag to notify threads when it is time to stop. Note that
    // flag must be atomic in order to safely publisj changes cross thread
    // bounds
    std::atomic<bool> running;

    std::mutex _dont_work;
    // Thread that is accepting new connections
    pthread_t accept_thread;

    // Maximum number of client allowed to exists concurrently
    // on server, permits access only from inside of accept_thread.
    // Read-only
    uint16_t max_workers;

    // Port to listen for new connections, permits access only from
    // inside of accept_thread
    // Read-only
    uint32_t listen_port;

    // Mutex used to access connections list
    std::mutex connections_mutex;

    // Conditional variable used to notify waiters about empty
    // connections list
    std::condition_variable connections_cv;

    // Threads that are processing connection data, permits
    // access only from inside of accept_thread
    ThreadsStatus threads_status;
};

} // namespace Blocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_BLOCKING_SERVER_H
