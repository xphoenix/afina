#ifndef AFINA_NETWORK_WORKER_H
#define AFINA_NETWORK_WORKER_H

#include <string>
#include <uv.h>
#include <vector>

#include "memcached/Parser.h"

namespace Afina {
class Storage;
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
    Worker(std::shared_ptr<Afina::Storage> pStorage) : pStorage(pStorage) {}
    ~Worker() {}

    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;

    void Start(const struct sockaddr_storage &addr);
    void Stop();

protected:
    // Size of input buffer
    const static size_t ConnectionInputBufferSize = 64 * 1024L;

    // Size of output buffer
    const static size_t ConnectionOutputBufferSize = 64 * 1024L;

    /**
     *
     *
     */
    enum ConnectionState : uint8_t {
        // Command header expected, i.e input stream must be read until header end
        // marker found
        sRecvHeader,

        // Data block expected, i.e read until all neccessary bytes consumed
        sRecvData,

        // Connection is sending bytes now
        sSend,

        // Connection has been requested to shutdown. It still flys around as wasn't completely
        // cleanup yet.
        sClosed
    };

    /**
     * Holds information about single connection from the client
     */
    typedef struct Connection {
        // Extend UV stream handler to use Connection instance inside
        // libuv lib
        uv_stream_t handler;

        // Current connection state, defines how buffered data processed
        ConnectionState state;

        // Buffer for input
        char *input;

        // HOw many bytes in input buffer if already used
        size_t input_used;

        // How many bytes from input has been parsed already
        size_t input_parsed;

        // Output to be send to client
        char *output;

        // How many bytes in output buffer if already used
        size_t output_used;

        // State of the header parser
        Memcached::Parser parser;

        Connection() : input(nullptr), output(nullptr), input_used(0), input_parsed(0), output_used(0) {
            input = new char[ConnectionInputBufferSize];
            output = new char[ConnectionOutputBufferSize];
            parser.Reset();
        }

        ~Connection() {
            delete[] input;
            delete[] output;
        }
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
     * Execute last command readed from the connection.
     */
    void Execute(Connection &pconn, std::unique_ptr<Execute::Command> cmd);

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

    /**
     * Storage instance to execute commands on
     */
    std::shared_ptr<Afina::Storage> pStorage;
};

} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_WORKER_H
