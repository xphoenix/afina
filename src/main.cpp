#include <chrono>
#include <iostream>
#include <memory>
#include <uv.h>

#include <afina/Storage.h>
#include <afina/Version.h>

#include "network/Server.h"
#include "storage/MapBasedGlobalLockImpl.h"

typedef struct {
    std::shared_ptr<Afina::Storage> storage;
    std::shared_ptr<Afina::Network::Server> server;
} Application;

// Handle all signals catched
void signal_handler(uv_signal_t *handle, int signum) {
    Application *pApp = static_cast<Application *>(handle->data);

    std::cout << "Receive stop signal" << std::endl;
    uv_stop(handle->loop);
}

// Called when it is time to collect passive metrics from services
void timer_handler(uv_timer_t *handle) {
    Application *pApp = static_cast<Application *>(handle->data);
    std::cout << "Start passive metrics collection" << std::endl;
}

int main(int argc, char **argv) {
    std::cout << "Starting Afina " << Afina::Version_Major << "." << Afina::Version_Minor << "."
              << Afina::Version_Patch;
    if (Afina::Version_SHA.size() > 0) {
        std::cout << "-" << Afina::Version_SHA;
    }
    std::cout << std::endl;

    // Build new storage instance
    Application app;
    app.storage = std::make_shared<Afina::Backend::MapBasedGlobalLockImpl>();

    // Build  & start network layer
    app.server = std::make_shared<Afina::Network::Server>(app.storage);

    // Init local loop. It will react to signals and performs some metrics collections. Each
    // subsystem is able to push metrics actively, but some metrics could be collected only
    // by polling, so loop here will does that work
    uv_loop_t loop;
    uv_loop_init(&loop);

    uv_signal_t sig;
    uv_signal_init(&loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGTERM | SIGKILL);
    sig.data = &app;

    uv_timer_t timer;
    uv_timer_init(&loop, &timer);
    timer.data = &app;
    uv_timer_start(&timer, timer_handler, 0, 5000);

    // Start services
    try {
        app.storage->Start();
        app.server->Start(8080);

        // Freeze current thread and process events
        std::cout << "Application started" << std::endl;
        uv_run(&loop, UV_RUN_DEFAULT);

        // Stop services
        app.server->Stop();
        app.server->Join();
        app.storage->Stop();

        std::cout << "Application stopped" << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Fatal error" << e.what() << std::endl;
    }

    return 0;
}
