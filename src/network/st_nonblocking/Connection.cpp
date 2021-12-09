#include "Connection.h"

#include <iostream>

namespace Afina {
namespace Network {
namespace STnonblock {

// See Connection.h
void Connection::Start() { 
    _event.events = EPOLLIN;
}

// See Connection.h
void Connection::OnError() { 
    isalive=false;
}

// See Connection.h
void Connection::OnClose() { 
    isalive=false;
}

// See Connection.h
void Connection::DoRead() {  
    try {
        int readed_bytes = -1;
        if ((readed_bytes = read(_socket, client_buffer+read_offset, sizeof(client_buffer))) > 0) {
            _logger->debug("Got {} bytes from socket", readed_bytes);

            // Single block of data readed from the socket could trigger inside actions a multiple times,
            // for example:
            // - read#0: [<command1 start>]
            // - read#1: [<command1 end> <argument> <command2> <argument for command 2> <command3> ... ]
            while (readed_bytes > 0) {
                _logger->debug("Process {} bytes", readed_bytes);
                // There is no command yet
                if (!command_to_execute) {
                    std::size_t parsed = 0;
                    if (parser.Parse(client_buffer, readed_bytes, parsed)) {
                        // There is no command to be launched, continue to parse input stream
                        // Here we are, current chunk finished some command, process it
                        _logger->debug("Found new command: {} in {} bytes", parser.Name(), parsed);
                        command_to_execute = parser.Build(arg_remains);
                        if (arg_remains > 0) {
                            arg_remains += 2;
                        }
                    }

                    // Parsed might fails to consume any bytes from input stream. In real life that could happens,
                    // for example, because we are working with UTF-16 chars and only 1 byte left in stream
                    if (parsed == 0) {
                        read_offset+=readed_bytes;
                        break;
                    } else {
                        std::memmove(client_buffer, client_buffer+read_offset + parsed, (read_offset+readed_bytes) - (read_offset+parsed));
                        read_offset=0;
                        readed_bytes -= parsed;
                    }
                }

                

                // There is command, but we still wait for argument to arrive...
                if (command_to_execute && arg_remains > 0) {
                    _logger->debug("Fill argument: {} bytes of {}", readed_bytes, arg_remains);
                    // There is some parsed command, and now we are reading argument
                    std::size_t to_read = std::min(arg_remains, std::size_t(readed_bytes));
                    argument_for_command.append(client_buffer, to_read);

                    std::memmove(client_buffer, client_buffer + to_read, readed_bytes - to_read);
                    arg_remains -= to_read;
                    readed_bytes -= to_read;
                }

                // Thre is command & argument - RUN!
                if (command_to_execute && arg_remains == 0) {
                    _logger->debug("Start command execution");

                    std::string result;
                    if (argument_for_command.size()) {
                        argument_for_command.resize(argument_for_command.size() - 2);
                    }
                    command_to_execute->Execute(*pStorage, argument_for_command, result);

                    // Send response
                    result += "\r\n";
                    _what_to_write_to_client_queue.emplace(result);
                    if (_what_to_write_to_client_queue.size()==max_size_of_to_client_queue){
                        _event.events &= ~EPOLLIN;
                    }
                    _event.events |= EPOLLOUT;

                    // Prepare for the next command
                    command_to_execute.reset();
                    argument_for_command.resize(0);
                    parser.Reset();
                }
            } // while (readed_bytes)
        }
        else{
            if (readed_bytes == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) { isalive = false; }

        }
        

    } 
    catch (std::runtime_error &ex) { //failed to parse command
        _logger->error("Failed to process connection on descriptor {}: {}", _socket, ex.what());
        OnClose();

    }

}

// See Connection.h
void Connection::DoWrite() {  
    while (_what_to_write_to_client_queue.size() > max_size_of_to_client_queue / 2 ) {
        const std::string& tek_elem = _what_to_write_to_client_queue.front();
        int sent_bytes = write(_socket, &tek_elem[write_offset], tek_elem.size() - write_offset);
        if (sent_bytes > 0) {
            write_offset += sent_bytes;
            if (write_offset == tek_elem.size()) {
                write_offset = 0;
                _what_to_write_to_client_queue.pop();
            }
        }
        else if (sent_bytes == EWOULDBLOCK) { return; }
        else {OnClose(); return;}
    }
    _event.events |= EPOLLIN;

    while (!_what_to_write_to_client_queue.empty() ) {
        const std::string& tek_elem = _what_to_write_to_client_queue.front();
        int sent_bytes = write(_socket, &tek_elem[write_offset], tek_elem.size() - write_offset);
        if (sent_bytes > 0) {
            write_offset += sent_bytes;
            if (write_offset == tek_elem.size()) {
                write_offset = 0;
                _what_to_write_to_client_queue.pop();
            }
        }
        else if (sent_bytes == EWOULDBLOCK) { return; }
        else {OnClose(); return;}
    }
    
    _event.events &= ~EPOLLOUT;
}

} // namespace STnonblock
} // namespace Network
} // namespace Afina
