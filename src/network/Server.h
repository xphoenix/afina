#ifndef AFINA_NETWORK_SERVER_H
#define AFINA_NETWORK_SERVER_H

#include <memory>
#include <vector>

#include "Worker.h"

namespace Afina {
class Storage;
namespace Network {

/**
 * # Network processors coordinator
 * Configure resources for the network processors and coordinates all work
 */
class Server {
public:
    Server(std::shared_ptr<Afina::Storage> ps) : pStorage(ps) {}
    ~Server() {}

    void Start(uint32_t port, uint16_t workers = 1);
    void Stop();
    void Join();

private:
    std::shared_ptr<Afina::Storage> pStorage;
    std::vector<Worker *> workers;
};

} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_SERVER_H
