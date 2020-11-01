#ifndef AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H

#include <cstring>
#include <iostream>
#include <vector>

#include <afina/Storage.h>
#include <afina/logging/Service.h>
#include <afina/execute/Command.h>
#include <protocol/Parser.h>
#include <sys/epoll.h>

namespace Afina {
namespace Network {
namespace STnonblock {

class Connection {
public:
    Connection(int s, std::shared_ptr<spdlog::logger> l, std::shared_ptr<Afina::Storage> stg)
    : _socket(s), _logger(l), pStorage(stg) {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }

    inline bool isAlive() const { return _is_alive; }

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
    bool _is_alive;

    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<Afina::Storage> pStorage;
    std::unique_ptr<Afina::Execute::Command> command_to_execute;
    std::vector<std::string> _results;
    Protocol::Parser parser;
    std::size_t arg_remains;
    std::string argument_for_command;

    int _written_bytes;
    int _read_bytes;
    int _bytes_for_read;
    char client_buffer[4096];
};

} // namespace STnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H