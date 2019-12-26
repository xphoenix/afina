#include "Connection.h"
#include <cassert>
#include <iostream>

namespace Afina {
namespace Network {
namespace STnonblock {

// See Connection.h
void Connection::Start() {
    _event.events = EPOLLIN;
    _alive = true;
    _need_to_close = false;
    _arg_remains = 0;
    _parser = Protocol::Parser{};
    _argument_for_command = "";
    _command_to_execute = nullptr;
    _off_set = 0;
    readed_bytes = 0;
}

// See Connection.h
void Connection::OnError() {
    std::cout << "void Connection::OnError()" << std::endl;
    _alive = false;
    shutdown(_socket, SHUT_RDWR);
    _command_to_execute.reset();
    _argument_for_command.resize(0);
    _parser.Reset();
    _answers.clear();
}

// See Connection.h
void Connection::OnClose() {
    std::cout << "void Connection::OnClose()" << std::endl;
    if (!_answers.empty()) {
        _event.events = EPOLLOUT;
        shutdown(_socket, SHUT_RD);
        _command_to_execute.reset();
        _argument_for_command.resize(0);
        _parser.Reset();
        _need_to_close = true;
    } else {
        OnError();
    }
}

// See Connection.h
void Connection::DoRead() {
    bool got_smth_to_write = false;
    // std::cout << _socket << std::endl;
    try {
        int cur_readed_bytes = -1;
        while ((cur_readed_bytes = read(_socket, client_buffer + readed_bytes, sizeof(client_buffer) - readed_bytes)) > 0) {
            readed_bytes += cur_readed_bytes;
            // _logger->debug("Got {} bytes from socket", readed_bytes);
            while (readed_bytes > 0) {
                // _logger->debug("Process {} bytes", readed_bystes);
                // There is no command yet
                if (!_command_to_execute) {
                    std::size_t parsed = 0;
                    if (_parser.Parse(client_buffer, readed_bytes, parsed)) {
                        // There is no command to be launched, continue to parse input stream
                        // Here we are, current chunk finished some command, process it
                        // _logger->debug("Found new command: {} in {} bytes", _parser.Name(), parsed);
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
                        std::memmove(client_buffer, client_buffer + parsed, readed_bytes - parsed);
                        readed_bytes -= parsed;
                    }
                }

                // There is command, but we still wait for argument to arrive...
                if (_command_to_execute && _arg_remains > 0) {
                    // _logger->debug("Fill argument: {} bytes of {}", readed_bytes, _arg_remains);
                    // There is some parsed command, and now we are reading argument
                    std::size_t to_read = std::min(_arg_remains, std::size_t(readed_bytes));
                    _argument_for_command.append(client_buffer, to_read);

                    std::memmove(client_buffer, client_buffer + to_read, readed_bytes - to_read);
                    _arg_remains -= to_read;
                    readed_bytes -= to_read;
                }

                // Thre is command & argument - RUN!
                if (_command_to_execute && _arg_remains == 0) {
                    // _logger->debug("Start command execution");

                    std::string result;
                    _command_to_execute->Execute(*_ps, _argument_for_command, result);

                    // Send response
                    result += "\r\n";
                    if (_answers.empty()){
                        got_smth_to_write = true;
                    }
                    _answers.push_back(result);

                    // Prepare for the next command
                    _command_to_execute.reset();
                    _argument_for_command.resize(0);
                    _parser.Reset();
                }
            } // while (readed_bytes)
        }

        if (readed_bytes == 0) {
            // _logger->debug("Connection closed");
            OnClose();
        }
        // else {
            // throw std::runtime_error(std::string(strerror(errno)));
        // }
    } catch (std::runtime_error &ex) {
        // _logger->error("Failed to process connection on descriptor {}: {}", _socket, ex.what());
        // SERVER_ERROR <описание>\r\n
        std::string msg = "SERVER_ERROR ";
        msg += ex.what();
        msg += "\r\n";
        if (_answers.empty()){
            got_smth_to_write = true;
        }
        _answers.push_back(msg);
        OnClose();
    }

    if (got_smth_to_write) {
        _event.events = EPOLLOUT;
    }
}

// See Connection.h
void Connection::DoWrite() {
    assert(_answers.size() > 0);
    std::size_t size = _answers.size() > 64 ? 64 : _answers.size();
    iovec data_to_write[size];
    for (std::size_t i = 0; i < size; i++) {
        data_to_write[i].iov_base = (void *)_answers[i].data(); //[i][0];
        data_to_write[i].iov_len = _answers[i].size();
    }
    data_to_write[0].iov_base = &_answers[0][_off_set];
    data_to_write[0].iov_len -= _off_set;

    std::size_t count = writev(_socket, data_to_write, size);

    if (count > 0) {
        std::size_t writed_bytes = _off_set + count;
        auto it = _answers.begin();
        while (((*it).size() <= writed_bytes) && (it != _answers.end())){
            writed_bytes -= (*it).size();
            it++;
        }
        _off_set = writed_bytes;
        _answers.erase(_answers.begin(), it);
    } else if (count == -1 && errno != EINTR) {
        OnError();
    }

    if (_answers.empty() && !_need_to_close) {
        _event.events = EPOLLIN;
    } else if (_answers.empty() && _need_to_close) {
        OnClose();
    }
}

} // namespace STnonblock
} // namespace Network
} // namespace Afina
