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
    Connection(int s, std::shared_ptr<Afina::Storage> ps) : _socket(s), _ps(ps) {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }

    ~Connection() { OnError(); }

    inline bool isAlive() const { return _alive; }

    void Start();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    friend class ServerImpl;

    int _socket;
    struct epoll_event _event;
    bool _alive;
    bool _need_to_close;

    std::shared_ptr<Afina::Storage> _ps;
    std::size_t _arg_remains;
    Protocol::Parser _parser;
    std::string _argument_for_command;
    std::unique_ptr<Execute::Command> _command_to_execute;

    char client_buffer[4096];
    int readed_bytes;

    std::deque<std::string> _answers;
    std::size_t _off_set;
};

} // namespace STnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
