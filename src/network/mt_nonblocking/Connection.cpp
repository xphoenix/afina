#include "Connection.h"

#include <iostream>

namespace Afina {
namespace Network {
namespace MTnonblock {

// See Connection.h
void Connection::Start() { std::cout << "Start" << std::endl; }

// See Connection.h
void Connection::OnError() { std::cout << "OnError" << std::endl; }

// See Connection.h
void Connection::OnClose() { std::cout << "OnClose" << std::endl; }

// See Connection.h
void Connection::DoRead() { std::cout << "DoRead" << std::endl; }

// See Connection.h
void Connection::DoWrite() { std::cout << "DoWrite" << std::endl; }

} // namespace MTnonblock
} // namespace Network
} // namespace Afina
