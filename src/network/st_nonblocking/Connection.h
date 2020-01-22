#ifndef AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H

#include <cstring>
#include <deque>
#include <memory>
#include <string>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include "protocol/Parser.h"
#include <afina/Storage.h>
#include <afina/execute/Command.h>

namespace Afina {
namespace Network {
namespace STnonblock {

class Connection {
public:
    Connection(int s, std::shared_ptr<Afina::Storage> pStorage) : _socket(s), _pStorage(pStorage) {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }

    ~Connection() {
        _command_to_execute.reset();
        _argument_for_command.resize(0);
        _parser.Reset();
        _responses.clear();
    }

    inline bool isAlive() const { return _is_alive; }

    void Start();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    static const uint32_t EVENT_READ = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET;
    static const uint32_t EVENT_WRITE = EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET;
    friend class ServerImpl;

    int _socket;
    struct epoll_event _event;

    bool _is_alive;
    std::size_t _offset;
    std::deque<std::string> _responses;
    int _readed_bytes;
    bool _wanna_be_closed;
    char _client_buffer[4096];

    std::shared_ptr<Afina::Storage> _pStorage;

    Protocol::Parser _parser;
    std::unique_ptr<Execute::Command> _command_to_execute;
    std::size_t _arg_remains;
    std::string _argument_for_command;
};

} // namespace STnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
