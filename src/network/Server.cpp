#include "Server.h"

#include <iostream>
#include <sys/mman.h>

#include <afina/Storage.h>

namespace Afina {
namespace Network {

void Server::Start(uint32_t port, uint16_t n_workers) {
    struct sockaddr_storage address;
    int rc = uv_ip4_addr("0.0.0.0", port, (struct sockaddr_in *)&address);
    if (rc != 0) {
        std::cerr << "Failed to call uv_ip4_addr: [" << uv_err_name(rc) << "(" << rc << ")]: " << uv_strerror(rc)
                  << std::endl;
        throw std::runtime_error("Failed to call uv_ip4_addr");
    }

    for (auto i = 0; i < n_workers; i++) {
        workers.push_back(new Worker(pStorage));
        workers[i]->Start(address);
    }
}

void Server::Stop(bool await_termination) {}

} // namespace Network
} // namespace Afina
