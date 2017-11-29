#ifndef AFINA_NETWORK_NONBLOCKING_WORKER_H
#define AFINA_NETWORK_NONBLOCKING_WORKER_H


#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signalfd.h>
#include <errno.h>

#include <memory>
#include <mutex>
#include <atomic>
#include <exception>
#include <list>
#include <iostream>
#include <cstring>
#include <map>
#include <memory>

#include <protocol/Parser.h>
//#include <afina/Executor.h>
#include <afina/execute/Command.h>
#include "Utils.h"

namespace Afina {

// Forward declaration, see afina/Storage.h
class Storage;

namespace Network {
namespace epoll {

/**
 * # Thread running epoll
 * On Start spaws background thread that is doing epoll on the given server
 * socket and process incoming connections and its data
 */
class Worker {
public:
    Worker(std::shared_ptr<Afina::Storage> ps);
    ~Worker();

    /**
     * Spaws new background thread that is doing epoll on the given server
     * socket. Once connection accepted it must be registered and being processed
     * on this thread
     */
    void Start(int server_socket);

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
    void OnRun(void *args);
    static void* RunProxy(void* args);
private:
    pthread_t thread;
    std::shared_ptr<Afina::Storage> ps;
    std::atomic<bool> running;
};

} // namespace NonBlocking
} // namespace Network
} // namespace Afina
#endif // AFINA_NETWORK_NONBLOCKING_WORKER_H
