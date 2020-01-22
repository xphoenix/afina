#include "Connection.h"

#include <cassert>
#include <iostream>

namespace Afina {
namespace Network {
namespace STnonblock {

// See Connection.h
void Connection::Start() {
    _event.events = EVENT_READ;
    _is_alive = true;
    _readed_bytes = 0;
    _command_to_execute = nullptr;
    _parser = Protocol::Parser{};
    _argument_for_command = "";
    _responses.clear();
    _arg_remains = 0;
    _wanna_be_closed = false;
}

// See Connection.h
void Connection::OnError() {
    _is_alive = false;
    _command_to_execute.reset();
    _argument_for_command.resize(0);
    _parser.Reset();
    _responses.clear();
}

// See Connection.h
void Connection::OnClose() {
    if (!_responses.empty()) {
        _event.events = EVENT_WRITE;
        _wanna_be_closed = true;
    } else {
        _is_alive = false;
    }
    _command_to_execute.reset();
    _argument_for_command.resize(0);
    _parser.Reset();
}

// See Connection.h
void Connection::DoRead() {
    try {
        int readed_bytes_current = -1;
        while ((readed_bytes_current =
                    read(_socket, _client_buffer + _readed_bytes, sizeof(_client_buffer) - _readed_bytes)) > 0) {
            _readed_bytes += readed_bytes_current;
            while (_readed_bytes > 0) {
                // There is no command yet
                if (!_command_to_execute) {
                    std::size_t parsed = 0;
                    if (_parser.Parse(_client_buffer, _readed_bytes, parsed)) {
                        // There is no command to be launched, continue to parse input stream
                        // Here we are, current chunk finished some command, process it
                        _command_to_execute = _parser.Build(_arg_remains);
                        if (_arg_remains > 0) {
                            _arg_remains += 2;
                        }
                    }

                    // Parsed might fails to consume any bytes from input stream. In real life that could happens,
                    // for example, because we are working with UTF-16 chars and only 1 byte left in stream
                    if (parsed == 0) {
                        break;
                    } else {
                        std::memmove(_client_buffer, _client_buffer + parsed, _readed_bytes - parsed);
                        _readed_bytes -= parsed;
                    }
                }

                // There is command, but we still wait for argument to arrive...
                if (_command_to_execute && _arg_remains > 0) {
                    // There is some parsed command, and now we are reading argument
                    std::size_t to_read = std::min(_arg_remains, std::size_t(_readed_bytes));
                    _argument_for_command.append(_client_buffer, to_read);

                    std::memmove(_client_buffer, _client_buffer + to_read, _readed_bytes - to_read);
                    _arg_remains -= to_read;
                    _readed_bytes -= to_read;
                }

                // There is command & argument - RUN!
                if (_command_to_execute && _arg_remains == 0) {
                    std::string result;
                    _command_to_execute->Execute(*_pStorage, _argument_for_command, result);

                    // Send response
                    result += "\r\n";

                    _responses.push_back(result);
                    _event.events |= EVENT_WRITE;

                    // Prepare for the next command
                    _command_to_execute.reset();
                    _argument_for_command.resize(0);
                    _parser.Reset();
                }
            } // while (_readed_bytes)
        }
        if ((!_is_alive) || (_readed_bytes == 0)) {
            _is_alive = false;
            _command_to_execute.reset();
            _argument_for_command.resize(0);
            _parser.Reset();
        }

    } catch (std::runtime_error &ex) {
        _command_to_execute.reset();
        _argument_for_command.resize(0);
        _parser.Reset();
        _responses.clear();
    }
}

// See Connection.h
void Connection::DoWrite() {
    assert(!_responses.empty());
    std::size_t resp_size = _responses.size() > 64 ? 64 : _responses.size();
    iovec resp_data[resp_size];
    auto iter = _responses.begin();
    for (auto &&data : resp_data) {
        if (iter != _responses.end()) {
            data.iov_base = (void *)iter->data();
            data.iov_len = iter->size();
            iter++;
        }
    }
    resp_data[0].iov_base = &(_responses[0][_offset]);
    resp_data[0].iov_len -= _offset;

    std::size_t count = writev(_socket, resp_data, resp_size);
    if (count > 0) {
        _offset += count;
        auto it = _responses.begin();
        while ((it != _responses.end()) && (it->size() <= _offset)) {
            _offset -= it->size();
            it++;
        }
        _responses.erase(_responses.begin(), it);
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        _is_alive = false;
    }

    if (_responses.empty() && !_wanna_be_closed) {
        _event.events = EVENT_READ;
    } else if (_responses.empty() && _wanna_be_closed) {
        _is_alive = false;

        _command_to_execute.reset();
        _argument_for_command.resize(0);
        _parser.Reset();
    }
}

} // namespace STnonblock
} // namespace Network
} // namespace Afina
