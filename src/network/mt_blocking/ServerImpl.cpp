#include "ServerImpl.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <spdlog/logger.h>

#include <afina/Storage.h>
#include <afina/execute/Command.h>
#include <afina/logging/Service.h>

#include "protocol/Parser.h"

namespace Afina {
namespace Network {
namespace MTblocking {

// See Server.h
ServerImpl::ServerImpl(std::shared_ptr<Afina::Storage> ps, std::shared_ptr<Logging::Service> pl) : Server(ps, pl) {}

// See Server.h
ServerImpl::~ServerImpl() {}

// See Server.h
void ServerImpl::Start(uint16_t port, uint32_t n_accept, uint32_t n_workers) {

	_max_workers = n_workers;

	_logger = pLogging->select("network");
	_logger->info("Start mt_blocking network service");

	sigset_t sig_mask;
	sigemptyset(&sig_mask);
	sigaddset(&sig_mask, SIGPIPE);
	if (pthread_sigmask(SIG_BLOCK, &sig_mask, nullptr) != 0) {
		throw std::runtime_error("Unable to mask SIGPIPE");
	}

	struct sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;         // IPv4
	server_addr.sin_port = htons(port);       // TCP port number
	server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any address

	_server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_server_socket == -1) {
		throw std::runtime_error("Failed to open socket");
	}

	int opts = 1;
	if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts)) == -1) {
		close(_server_socket);
		throw std::runtime_error("Socket setsockopt() failed");
	}

	if (bind(_server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		close(_server_socket);
		throw std::runtime_error("Socket bind() failed");
	}

	if (listen(_server_socket, 5) == -1) {
		close(_server_socket);
		throw std::runtime_error("Socket listen() failed");
	}

	_running.store(true);
	_thread = std::thread(&ServerImpl::OnRun, this);
}

// See Server.h
void ServerImpl::Stop() {
	_logger->debug("Stopping server");

	_running.store(false);
	shutdown(_server_socket, SHUT_RDWR);
	
	std::unique_lock<std::mutex> lock(mut);
	for (auto descriptor : _sockets) {
		shutdown(descriptor, SHUT_RD);
	}

	_logger->debug("Stopping executor");
}

// See Server.h
void ServerImpl::Join() {
	assert(_thread.joinable());
	_thread.join();
}

// See Server.h
void ServerImpl::OnRun() {
	while (_running.load()) {
		_logger->debug("Waiting for connection...");

		// The call to accept() blocks until the incoming connection arrives
		int client_socket;
		struct sockaddr client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		if ((client_socket = accept(_server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
			continue;
		}

		// Got new connection
		if (_logger->should_log(spdlog::level::debug)) {
			std::string host = "unknown", port = "-1";

			char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
			if (getnameinfo(&client_addr, client_addr_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
							NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
				host = hbuf;
				port = sbuf;
			}
			_logger->debug("Accepted connection on descriptor {} (host={}, port={})\n", client_socket, host, port);
		}

		// Configure read timeout
		{
			struct timeval tv;
			tv.tv_sec = 5; // TODO: make it configurable
			tv.tv_usec = 0;
			setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
		}


		{
			std::unique_lock<std::mutex> l(mut);
			if (_sockets.size() < _max_workers && _running.load()) {
				try {
					// running new worker
					std::thread(&ServerImpl::worker, this, client_socket).detach();
					// add new descriptor to the set
					_sockets.emplace(client_socket);

				} catch (...) {
					close(client_socket);
					throw std::runtime_error("Unable to start worker. Closing client socket");
				}
			}
		}
	}

	// Cleanup on exit...

	close(_server_socket);
	std::unique_lock<std::mutex> l(mut);
	_check_current_workers.wait(l, 
		[this] { 
			return this->_sockets.empty(); 
		}

	);

	_logger->warn("Network stopped");
}

void ServerImpl::worker(int client_socket) {
	// Here is connection state
	// - parser: parse state of the stream
	// - command_to_execute: last command parsed out of the stream
	// - arg_remains: how many bytes to read from stream to get command argument
	// - argument_for_command: buffer containing argument
	std::size_t arg_remains = 0;
	Protocol::Parser parser;
	std::string argument_for_command;
	std::string response;
	std::unique_ptr<Execute::Command> command_to_execute;

	// Process new connection:
	// 1) Read commands while socket is alive
	// 2) Execute commands
	// 3) Send response
	try {
		int bytes_read;
		char buf[4096] = "";
		while ((bytes_read = read(client_socket, buf, sizeof(buf))) > 0) {
			_logger->debug("Got {} bytes from socket", bytes_read);

			// Single block of data read from the socket may invoke multiple actions
			while (bytes_read > 0) {
				_logger->debug("Processing {} bytes", bytes_read);
				
				// No command yet
				if (!command_to_execute) {
					std::size_t parsed = 0;
					if (parser.Parse(buf, bytes_read, parsed)) {
						// Command found
						_logger->debug("Found new command: {} in {} bytes", parser.Name(), parsed);
						command_to_execute = parser.Build(arg_remains);
						if (arg_remains > 0) {
							arg_remains += 2;
						}
					}

					// Parsing might fail to read any bytes from the input stream. For example, that might happen
					// when we use UTF-16 chars and there is only 1 byte left in the stream
					if (parsed == 0) {
						break;
					} else {
						std::memmove(buf, buf + parsed, bytes_read - parsed);
						bytes_read -= parsed;
					}
				}

				// Reading remaining arguments
				if (command_to_execute && arg_remains > 0) {
					_logger->debug("Receiving arguments: {} bytes of {}", bytes_read, arg_remains);
					
					std::size_t to_read = std::min(arg_remains, std::size_t(bytes_read));
					argument_for_command.append(buf, to_read);

					std::memmove(buf, buf + to_read, bytes_read - to_read);
					arg_remains -= to_read;
					bytes_read -= to_read;
				}

				// Command and arguments received. Running
				if (command_to_execute && arg_remains == 0) {
					_logger->debug("Starting command execution");

					command_to_execute->Execute(*pStorage, argument_for_command, response);
					response += "\r\n";

					if (send(client_socket, response.data(), response.size(), 0) <= 0) {
						throw std::runtime_error("Failed to send response");
					}

					// Prepare for the next command
					command_to_execute.reset();
					argument_for_command.resize(0);
					response.resize(0);
					parser.Reset();
				}

			} // while (bytes_read > 0)
		}

		if (bytes_read == 0) {
			_logger->debug("Connection closed");
		} else {
			throw std::runtime_error(std::string(strerror(errno)));
		}
	} catch (std::runtime_error &ex) {
		_logger->error("Failed to process connection on descriptor {}: {}", client_socket, ex.what());
	}

	// Closing connection

	std::lock_guard<std::mutex> lock(mut);
	close(client_socket);
	_sockets.erase(client_socket);
	
	if (!_running.load() && _sockets.empty()) {
		_check_current_workers.notify_one();
	}
	
}

} // namespace MTblocking
} // namespace Network
} // namespace Afina