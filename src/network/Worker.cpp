#include "Worker.h"

#include <arpa/inet.h>
#include <cassert>
#include <sstream>
#include <stdexcept>

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

void Worker::Stop() {}

// Run method of the event loop thread, it must setup thread local resources, and launch event loop.
// Once loop terminated, method cleans up all local resources
void Worker::OnRun() {
    // Run network loop, that call won't return until event loop shuted down by libuv
    // routines
    uv_run(&uvLoop, UV_RUN_DEFAULT);

    // TODO: Here is cleanup logic
}

// Called once signal from outside world received that it is time to stop the network layer.
// Method should perform graceful stop and wait until all existing connections/jobs are complete
// before actually terminate the loop
void Worker::OnStop(uv_async_t *async) {
    // Other calls to async has no meaning
    // TODO: Shutdown
    // if (uv_is_closing((uv_handle_t *)&uvStopAsync) == 0) {
    //     uv_close((uv_handle_t *)&uvStopAsync, delegate<Worker>::callback<&Worker::OnHandleClosed>);
    // }
}

// Called when some handle has been closed, connection, write request and task handlers has special
// callback, that one is used for async & server socket handler
void Worker::OnHandleClosed(uv_handle_t *) {
    // TODO: Track all open handlers here to signal stop routine once shutdown is possible
    // logger->info("Handler closed");
    // openHandlersCount--;
    // StoppedIfRequestedAndPossible();
}

// New connection arrived in the server socket, here resources for connection get allocated and
// reading begin. According to our protocol client just start sending new commands, so server is
// always reacts to what it gets
void Worker::OnConnectionOpen(uv_stream_t *server, int status) {
    // Allocate new connection from the memory pool
    Connection *pconn = new Connection;

    // Init connection
    uv_tcp_init(&uvLoop, (uv_tcp_t *)pconn);
    pconn->handler.data = this;
    pconn->parsed = 0;

    // Setup client socket
    int rc = uv_accept(server, (uv_stream_t *)pconn);
    if (rc != 0) {
        std::stringstream ss;
        ss << "Failed to call uv_thread_create: [" << uv_err_name(rc) << ", " << rc << "]: " << uv_strerror(rc);
        throw std::runtime_error(ss.str());
    }

    // TODO: track open handler here
}

// Called once underlaying socket closed, used to delete all resources associated with connection
void Worker::OnConnectionClose(uv_handle_t *conn) {
    Connection *pconn = (Connection *)(conn);
    // TODO: Connection closed, discard all queued commands, mark connection as closed to discard all
    // running commands results and delete pointer once it has no commands
}

// Just before read, libuv calls that method to allocate some memory chunk where read copies socket data.
// Connection shares one input buffer which is built on top of slab allocator, we are always allocate memory
// there until all incoming data fits, parse it and deallocate buffer just after that.
void Worker::OnAllocate(uv_handle_t *conn, size_t suggested_size, uv_buf_t *buf) {
    assert(conn);
    Connection *pconn = (Connection *)(conn);

    // Allocate buffer but do not increase write position as there is no guarantee
    // how many bytes it will be possible to read from the socket
    //
    // ibuf takes care of defagmentation, so if in 16KB buffer only last few bytes
    // are used, code below will copy that bytes to begin before allocate write chunk
    //
    // Because of that it is possible to use buffer just like large, continues area in
    // memory between here and OnRead method
    pconn->input.reserve(4 * 1024L);
    size_t len = pconn->input.size();
    buf->base = &pconn->input[len];
    buf->len = 4 * 1024L;
}

// Once soket is ready to give some bytes back to application libuv calls that method,
// buf that we received points somewere inside connection input buffer, so once some
// data read, pconn->in writer position must be updated
void Worker::OnRead(uv_stream_t *conn, ssize_t nread, const uv_buf_t *buf) {
    assert(conn);
    Connection *pconn = (Connection *)(conn);

    // negative nread indicates that socket has been closed
    if (nread < 0) {
        uv_close((uv_handle_t *)(pconn), delegate<Worker>::callback<&Worker::OnConnectionClose>);
        return;
    }

    // Update string state to reflect new arrived data
    size_t len = pconn->input.size();
    pconn->input.resize(len + nread);

    // Look for the command delimeters in the [parsed, input.size()). Note that buffer could contains
    // many commands, not only one
    // TODO: Parse commands
}

size_t Worker::Parse(const std::string &str, Execute::Command *out) { return 0; }

void Worker::Execute(Connection &pconn) {}

} // namespace Network
} // namespace Afina
