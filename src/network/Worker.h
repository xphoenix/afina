#ifndef AFINA_NETWORK_WORKER_H
#define AFINA_NETWORK_WORKER_H

#include <string>
#include <uv.h>
#include <vector>

namespace Afina {
namespace Execute {
class Command;
}
namespace Network {

/**
 * # Basic network data processor
 * Reads and writes byte streams from/to clients, parse protocol and submit commands to the execution. Implements
 * logic protocol
 */
class Worker {
public:
    Worker() {}
    ~Worker() {}

    void Start(const struct sockaddr_storage &addr);
    void Stop();

protected:
    /**
     * Holds information about single connection from the client
     */
    typedef struct {
        // Extend UV stream handler to use Connection instance inside
        // libuv lib
        uv_stream_t handler;

        // Buffer for input
        std::string input;

        // How many bytes from the buffer is already parsed
        size_t parsed;
    } Connection;

    /**
     * Called by thread once started, while this method is running Worker considered as alive
     */
    void OnRun();

    /**
     * Called once external thread request worker to stop
     */
    void OnStop(uv_async_t *);

    /**
     * Called once some libuv handler is closed and could be removed
     */
    void OnHandleClosed(uv_handle_t *);

    /**
     * Called by UV once new connection arrived
     */
    void OnConnectionOpen(uv_stream_t *, int);

    /**
     * Called by UV when connection gets closed by some reason
     */
    void OnConnectionClose(uv_handle_t *);

    /**
     * LibUV used that method just before call OnRead to allocate temporary buffer
     * for the input
     */
    void OnAllocate(uv_handle_t *, size_t suggested_size, uv_buf_t *buf);

    /**
     * Connection is ready to read data
     */
    void OnRead(uv_stream_t *, ssize_t nread, const uv_buf_t *buf);

    /**
     * Parse given command. If more data needs to be read from the underlaying connection then method
     * returns how many bytes need.If no additional data needs then method returns 0
     *
     * If buffer contains valid serialized command then it should be constructed and put into gicven pointer
     */
    size_t Parse(const std::string &str, Execute::Command *out);

    /**
     * Execute last command readed from the connection.
     */
    void Execute(Connection &pconn);

private:
    /**
     * Thread running current worker
     */
    uv_thread_t thread;

    /**
     * Loops used to process network events. All workers of the same server shares a single port
     * and use SO_SHAREDPROTO to let kernel distribute the load
     */
    uv_loop_t uvLoop;

    /**
     * Noop handler for sigpipe signal
     */
    uv_signal_t uvSigPipe;

    /**
     * TCP/IP socket used by server to listen for incomming connection
     */
    uv_tcp_t uvNetwork;

    /**
     * List of all "alive" connections, some of it could be in closed state, but can't be removed yet
     * due to running commands
     */
    std::vector<Connection *> alive;
};

} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_WORKER_H
