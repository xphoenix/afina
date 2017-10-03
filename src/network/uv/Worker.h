#ifndef AFINA_NETWORK_UV_WORKER_H
#define AFINA_NETWORK_UV_WORKER_H

#include <string>
#include <unordered_set>
#include <uv.h>
#include <vector>

#include <afina/execute/Command.h>
#include <protocol/Parser.h>

namespace Afina {
class Storage;
namespace Execute {
class Command;
}
namespace Network {
namespace UV {

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

    /**
     * Signal worker that  it should stop. Method returns immediately, after that
     * all new incomming connections will be rejected, currently readed commands complete
     * and existing connections closed.
     *
     * Once all existing connections closed worker will stop
     */
    void Stop();

    /**
     * Blocks calling thread until worker is stopped. If worker hasn't been started then
     * method return immediately
     */
    void Join();

protected:
    // Size of input buffer
    const static size_t ConnectionInputBufferSize = 64 * 1024L;

    // Determinates how connection reacts on different async events, such as
    // new input data or command execution complete
    enum ConnectionState : uint8_t {
        // Command header expected, i.e input stream must be read until header end
        // marker found
        sRecvHeader,

        // Data block expected, i.e read until all neccessary bytes consumed
        sRecvBody,

        // Data block received and \r trailer expected
        sRecvTrailerCR,

        // Data block received and \n trailer expected
        sRecvTrailerLF,

        // Command was parsed out, time to execute it
        sExecute,

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

        // State of the header parser
        Protocol::Parser parser;

        // Command parsed out from the input
        std::unique_ptr<Execute::Command> cmd;

        // Number of bytes left to read to get command
        uint32_t body_size;

        // Argument for the command
        std::string body;

        // Number of tasks that are running now
        size_t runningTasks;

        Connection()
            : state(ConnectionState::sRecvHeader), input(nullptr), input_used(0), input_parsed(0), cmd(nullptr),
              body_size(0), body(""), runningTasks(0) {
            input = new char[ConnectionInputBufferSize];
            parser.Reset();
        }

        ~Connection() { delete[] input; }
    } Connection;

    /**
     * Work passed to the worker thread pool and back in order to execute
     * some command
     */
    typedef struct ExecuteTask {
        // Write handler, used to send this task through the libuv write pipeline
        uv_write_t handler;

        // Async signal to be called once task execution is complete
        uv_async_t done;

        // Connection that received command, used to write out response
        Connection *connection;

        // Command to execute
        std::unique_ptr<Execute::Command> cmd;

        // Argument for the command
        std::string argument;

        // Execution result
        uv_buf_t result;
    } ExecuteTask;

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
     * Check that all async resources allocated for network are free and close
     * event loop
     */
    void CloseEventLoppIfPossible();

    /**
     * Called by UV once new connection arrived
     */
    void OnConnectionOpen(uv_stream_t *, int);

    /**
     * Called by UV when connection gets closed by some reason
     */
    void OnConnectionClosed(uv_handle_t *);

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
     * Execute last command readed from the connection. Once method return all fields in connection allocated for the
     * command will be released, so implementation must take care to copy/move data somewhere else in case it needs
     * for a time  longer then function execution
     */
    void Execute(Connection &pconn);

    /**
     * Called once command execution is complete
     */
    void OnExecutionDone(uv_async_t *handle);

    /**
     * Called by libuv once ExecuteTask output buffer has been written to the output connection
     */
    void OnWriteDone(uv_write_t *req, int status);

private:
    // // State of worker, could transit only in one direction from left to right
    // enum class WorkerState : uint8_t { kInit, kRun, kStopping, kStopped };
    //
    // /**
    //  * State, defines what operations are allowed. Any access to this variable must be protected by stateLock,
    //  * and every change must notify stateChanges variable
    //  */
    // WorkerState state;

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
     * Async used by stop routine in order to wake up event loop
     */
    uv_async_t uvStopAsync;

    /**
     * TCP/IP socket used by server to listen for incomming connection
     */
    uv_tcp_t uvNetwork;

    /**
     * List of all "alive" connections, some of it could be in closed state, but can't be removed yet
     * due to running commands
     */
    std::unordered_set<Connection *> alive;

    /**
     * Storage instance to execute commands on
     */
    std::shared_ptr<Afina::Storage> pStorage;
};

} // namespace UV
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_UV_WORKER_H
