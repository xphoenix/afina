#ifndef AFINA_NETWORK_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_NONBLOCKING_CONNECTION_H

#include <cstring>

#include <sys/epoll.h>

namespace Afina {
namespace Network {
namespace NonBlocking {

class Connection {
public:
    Connection(int s) : _socket(s) { std::memset(&_event, 0, sizeof(struct epoll_event)); }

    inline bool isAlive() const { return true; }

    void Start();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    friend class Worker;
    friend class ServerImpl;

    int _socket;
    struct epoll_event _event;
};

} // namespace NonBlocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_NONBLOCKING_CONNECTION_H
