#include "Worker.h"

#include <iostream>

//#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "Utils.h"

namespace Afina {
namespace Network {
namespace epoll {




// See Worker.h
Worker::Worker(std::shared_ptr<Afina::Storage> ps) {
    // TODO: implementation here

    
    // pStorage = ps;
    // fifo_fd = -1;
}

// See Worker.h
Worker::~Worker() {
    // TODO: implementation here
}

void* Worker::RunProxy(void* _args) {
  auto args = reinterpret_cast<std::pair<Worker*, int>*>(_args);
  Worker* worker_instance = args->first;
  int server_socket = args->second;
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);
  if (pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1) {
    throw std::runtime_error("pthread_sigmask");
  }
  worker_instance->OnRun(server_socket);
  return NULL;
}


// See Worker.h
void Worker::Start(int server_socket) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    // TODO: implementation here
    this->running.store(true);

    //pthread_create get pair <key, object>
    auto args = new std::pair<Worker*, int> (this, server_socket);
    if (pthread_create( &(this->thread), NULL, &(Worker::RunProxy) , args) != 0){
        throw std::runtime_error("can't create a thread");
    }
}


// See Worker.h
void Worker::Stop() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    // TODO: implementation here

    // running.store(false);
}

// See Worker.h
void Worker::Join() {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;
    // TODO: implementation here

    // pthread_join(this->thread, ...)
}

// See Worker.h
void *Worker::OnRun(void *args) {
    std::cout << "network debug: " << __PRETTY_FUNCTION__ << std::endl;

    // TODO: implementation here
    // 1. Create epoll_context here
    // 2. Add server_socket to context
    // 3. Accept new connections, don't forget to call make_socket_nonblocking on
    //    the client socket descriptor
    // 4. Add connections to the local context
    // 5. Process connection events
    //
    // Do not forget to use EPOLLEXCLUSIVE flag when register socket
    // for events to avoid thundering herd type behavior.


    // while(running.load()) {

    // }
}

} // namespace NonBlocking
} // namespace Network
} // namespace Afina
