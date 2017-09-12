#include "Worker.h"

#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <afina/Storage.h>
#include <afina/execute/Command.h>

namespace Afina {
namespace Network {

/**
 * Template to generate various callback wrappers that translate c style callback into class
 * methods call
 */
// TODO: rewrite variadic template to use normal function signartures as ret(args...) and move
// callback template <T, T::*TMethod> into callback method
template <typename T, typename... Types> struct delegate {
    template <void (T::*TMethod)(uv_handle_t *, Types...)> static void callback(uv_handle_t *self, Types... args) {
        T *instance = static_cast<T *>(self->data);
        (instance->*TMethod)(self, std::forward<Types>(args)...);
    }

    template <void (T::*TMethod)(uv_stream_t *, Types...)> static void callback(uv_stream_t *self, Types... args) {
        T *instance = static_cast<T *>(self->data);
        (instance->*TMethod)(self, std::forward<Types>(args)...);
    }

    template <void (T::*TMethod)(uv_write_t *, Types...)> static void callback(uv_write_t *self, Types... args) {
        T *instance = static_cast<T *>(self->data);
        (instance->*TMethod)(self, std::forward<Types>(args)...);
    }

    template <void (T::*TMethod)(uv_work_t *, Types...)> static void callback(uv_work_t *self, Types... args) {
        T *instance = static_cast<T *>(self->data);
        (instance->*TMethod)(self, std::forward<Types>(args)...);
    }

    template <void (T::*TMethod)(uv_async_t *, Types...)> static void callback(uv_async_t *self, Types... args) {
        T *instance = static_cast<T *>(self->data);
        (instance->*TMethod)(self, std::forward<Types>(args)...);
    }

    template <void (T::*TMethod)(Types...)> static void callback(Types... args, void *data) {
        T *instance = static_cast<T *>(data);
        (instance->*TMethod)(std::forward<Types>(args)...);
    }
};

void noop(uv_signal_t *handle, int signum) {}

// See Worker.h
void Worker::Start(const struct sockaddr_storage &address) {
    // Init loop
    int rc = uv_loop_init(&uvLoop);
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to create loop: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }
    uvLoop.data = this;

    // Init signals
    rc = uv_signal_init(&uvLoop, &uvSigPipe);
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to call uv_signal_init: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }
    uv_signal_start(&uvSigPipe, noop, SIGPIPE);

    // Setup Network
    rc = uv_tcp_init_ex(&uvLoop, &uvNetwork, address.ss_family);
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to call uv_tcp_init_ex: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }
    uvNetwork.data = this;

    // Configure network
    int fd;
    rc = uv_fileno((uv_handle_t *)&uvNetwork, &fd);
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to call uv_fileno: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }

    rc = uv_tcp_keepalive(&uvNetwork, 1, 60);
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to call uv_tcp_keepalive: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }

    int on = 1;
    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to call setsockopt: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }

    rc = uv_tcp_bind(&uvNetwork, (const struct sockaddr *)&address, 0);
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to call uv_tcp_bind: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }

    rc = uv_listen((uv_stream_t *)&uvNetwork, 511, delegate<Worker, int>::callback<&Worker::OnConnectionOpen>);
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to call uv_listen: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }

    // Start thread
    rc = uv_thread_create(&thread, delegate<Worker>::callback<&Worker::OnRun>, static_cast<void *>(this));
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to call uv_thread_create: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }
}

// See Worker.h
void Worker::Stop() {}

// Run method of the event loop thread, it must setup thread local resources, and launch event loop.
// Once loop terminated, method cleans up all local resources
// See Worker.h
void Worker::OnRun() {
    // Run network loop, that call won't return until event loop shuted down by libuv
    // routines
    uv_run(&uvLoop, UV_RUN_DEFAULT);

    // TODO: Here is cleanup logic
}

// Called once signal from outside world received that it is time to stop the network layer.
// Method should perform graceful stop and wait until all existing connections/jobs are complete
// before actually terminate the loop
// See Worker.h
void Worker::OnStop(uv_async_t *async) {
    std::cout << "network debug:" << __PRETTY_FUNCTION__ << std::endl;
    // Other calls to async has no meaning
    // TODO: Shutdown
    // if (uv_is_closing((uv_handle_t *)&uvStopAsync) == 0) {
    //     uv_close((uv_handle_t *)&uvStopAsync, delegate<Worker>::callback<&Worker::OnHandleClosed>);
    // }
}

// Called when some handle has been closed, connection, write request and task handlers has special
// callback, that one is used for async & server socket handler
// See Worker.h
void Worker::OnHandleClosed(uv_handle_t *) {
    std::cout << "network debug:" << __PRETTY_FUNCTION__ << std::endl;
    // TODO: Track all open handlers here to signal stop routine once shutdown is possible
    // logger->info("Handler closed");
    // openHandlersCount--;
    // StoppedIfRequestedAndPossible();
}

// New connection arrived in the server socket, here resources for connection get allocated and
// reading begin. According to our protocol client just start sending new commands, so server is
// always reacts to what it gets
// See Worker.h
void Worker::OnConnectionOpen(uv_stream_t *server, int status) {
    std::cout << "network debug:" << __PRETTY_FUNCTION__ << std::endl;
    // Allocate new connection from the memory pool
    Connection *pconn = new Connection;
    alive.push_back(pconn);

    // Init connection
    uv_tcp_init(&uvLoop, (uv_tcp_t *)pconn);
    pconn->handler.data = this;

    // Setup client socket
    int rc = uv_accept(server, (uv_stream_t *)pconn);
    if (rc != 0) {
        std::cerr << "Failed to call uv_accept: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        uv_close((uv_handle_t *)(pconn), delegate<Worker>::callback<&Worker::OnConnectionClose>);
        return;
    }

    // Client driven protocol
    rc = uv_read_start((uv_stream_t *)pconn, delegate<Worker, size_t, uv_buf_t *>::callback<&Worker::OnAllocate>,
                       delegate<Worker, ssize_t, const uv_buf_t *>::callback<&Worker::OnRead>);
    if (rc != 0) {
        std::cerr << "Failed to call uv_read_start: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        uv_close((uv_handle_t *)(pconn), delegate<Worker>::callback<&Worker::OnConnectionClose>);
        return;
    }
}

// Called once underlaying socket closed, used to delete all resources associated with connection
// See Worker.h
void Worker::OnConnectionClose(uv_handle_t *conn) {
    std::cout << "network debug:" << __PRETTY_FUNCTION__ << std::endl;
    Connection *pconn = (Connection *)(conn);
    // TODO: Connection closed, discard all queued commands, mark connection as closed to discard all
    // running commands results and delete pointer once it has no commands
}

// Just before read, libuv calls that method to allocate some memory chunk where read copies socket data.
// We are always allocate memorythere until all incoming data fits, parse it and deallocate buffer just after that.
// See Worker.h
void Worker::OnAllocate(uv_handle_t *conn, size_t suggested_size, uv_buf_t *buf) {
    assert(conn);

    Connection *pconn = (Connection *)(conn);
    assert(pconn->input_parsed <= pconn->input_used);

    size_t unparsed = pconn->input_used - pconn->input_parsed;
    std::memmove(pconn->input, pconn->input + pconn->input_parsed, unparsed);

    pconn->input_parsed = 0;
    pconn->input_used = unparsed;

    buf->base = &pconn->input[unparsed];
    buf->len = ConnectionInputBufferSize - unparsed;
}

// Once soket is ready to give some bytes back to application libuv calls that method,
// buf that we received points somewere inside connection input buffer, so once some
// data read, pconn->in writer position must be updated
// See Worker.h
void Worker::OnRead(uv_stream_t *conn, ssize_t nread, const uv_buf_t *buf) {
    std::cout << "network debug:" << __PRETTY_FUNCTION__ << std::endl;
    assert(conn);
    Connection *pconn = (Connection *)(conn);

    // negative nread indicates that socket has been closed
    if (nread < 0) {
        uv_close((uv_handle_t *)(pconn), delegate<Worker>::callback<&Worker::OnConnectionClose>);
        return;
    }

    // Look for the command delimeters in the [parsed, input.size()). Note that buffer could contains
    // many commands, not only one
    pconn->input_used += nread;
    while (pconn->input_parsed < pconn->input_used) {
        // Read header or body if needs
        if (pconn->state == ConnectionState::sRecvHeader) {
            // Try to parse command out
            if (!pconn->parser.Parse(pconn->input, pconn->input_used, pconn->input_parsed)) {
                continue;
            }

            // Command has been parsed form input
            pconn->cmd = pconn->parser.Build(pconn->body_size);

            // Command has argument that needs to be read from the network connection before execution could take place
            if (pconn->body_size > 0) {
                pconn->body.clear();
                pconn->state = ConnectionState::sRecvBody;
            } else {
                pconn->state = ConnectionState::sExecute;
            }
        } else if (pconn->state == ConnectionState::sRecvBody) {
            size_t for_copy = std::min(uint32_t(pconn->input_used - pconn->input_parsed), pconn->body_size);
            pconn->body.append(pconn->input + pconn->input_parsed, for_copy);

            pconn->body_size -= for_copy;
            pconn->input_parsed += for_copy;

            if (pconn->body_size == 0) {
                pconn->state = ConnectionState::sRecvTrailerCR;
            }
        } else if (pconn->state == ConnectionState::sRecvTrailerCR) {
            if (pconn->input[pconn->input_parsed] != '\r') {
                throw std::runtime_error("Invalid chat, \\r expected");
            }
            pconn->input_parsed++;
            pconn->state = ConnectionState::sRecvTrailerLF;
        } else if (pconn->state == ConnectionState::sRecvTrailerLF) {
            if (pconn->input[pconn->input_parsed] != '\n') {
                throw std::runtime_error("Invalid chat, \\n expected");
            }
            pconn->input_parsed++;
            pconn->state = ConnectionState::sExecute;
        }

        if (pconn->state == ConnectionState::sExecute) {
            Execute(*pconn);

            pconn->cmd.reset();
            pconn->body.clear();
            pconn->parser.Reset();
            pconn->state = ConnectionState::sRecvHeader;
        }
    }
}

// See Worker.h
void Worker::Execute(Connection &pconn) {
    std::cout << "network debug:" << __PRETTY_FUNCTION__ << std::endl;

    // Setup execution params
    ExecuteTask *ptask = new ExecuteTask();
    ptask->connection = &pconn;
    ptask->cmd = std::move(pconn.cmd);
    ptask->argument = std::move(pconn.body);

    // Setup async signal to be called once task execution is complete
    int rc = uv_async_init(&uvLoop, &ptask->done, delegate<Worker>::callback<&Worker::OnExecutionDone>);
    if (rc != 0) {
        throw std::runtime_error("Failed to call uv_async_init for the task");
    }
    ptask->done.data = this;

    // TODO: That should be in another thread
    {
        std::string output;
        try {
            ptask->cmd->Execute(*pStorage, ptask->argument, output);
        } catch (std::runtime_error &ex) {
            std::cerr << "Failed to execute command: " << ex.what() << std::endl;

            std::stringstream ss;
            ss << "SERVER_ERROR " << ex.what();
            output = ss.str();
        }

        // Prepare output
        size_t size = output.size() + 2;
        ptask->result.base = new char[size];
        ptask->result.len = size;

        std::memcpy(ptask->result.base, &output[0], size - 2);
        ptask->result.base[size - 2] = '\r';
        ptask->result.base[size - 1] = '\n';

        // Notify event loop about task completition
        uv_async_send(&ptask->done);
    }
}

// See Worker.h
void Worker::OnExecutionDone(uv_async_t *handle) {
    std::cout << "network debug:" << __PRETTY_FUNCTION__ << std::endl;

    assert(handle);
    ExecuteTask *task = (ExecuteTask *)((uint8_t *)handle - offsetof(ExecuteTask, done));
    assert(&task->done == handle);

    // We don't need async anymore
    uv_close((uv_handle_t *)&task->done, delegate<Worker>::callback<&Worker::OnHandleClosed>);

    // Write command result to connection
    if (task->connection->state == ConnectionState::sClosed) {
        std::cerr << "Connection closed before command complete: ignore result" << std::endl;
    } else {
        // Send buffer to socket
        int rc = uv_write(&task->handler, &task->connection->handler, &task->result, 1,
                          delegate<Worker, int>::callback<&Worker::OnWriteDone>);
        if (rc != 0) {
            throw std::runtime_error("Failed to write request");
        }
    }
}

// See Worker.h
void Worker::OnWriteDone(uv_write_t *req, int status) {
    std::cout << "network debug:" << __PRETTY_FUNCTION__ << std::endl;
    assert(req != nullptr);
    ExecuteTask *task = (ExecuteTask *)req;
    Connection *pconn = task->connection;

    delete[] task->result.base;
    delete task;
}

} // namespace Network
} // namespace Afina
