#ifndef AFINA_NETWORK_UV_SERVER_H
#define AFINA_NETWORK_UV_SERVER_H

#include <memory>
#include <vector>

#include <afina/network/Server.h>

#include "Worker.h"

namespace Afina {
class Storage;
namespace Network {
namespace UV {

/**
 * # Network resource manager implementation
 * Implementation on top of lib uv library
 */
class ServerImpl : public Server {
public:
    ServerImpl(std::shared_ptr<Afina::Storage> ps);
    ~ServerImpl();

    // See Server.h
    void Start(uint32_t port, uint16_t workers) override;

    // See Server.h
    void Stop() override;

    // See Server.h
    void Join() override;

protected:
    /**
     * List of all workers created for this instance of server
     */
    std::vector<Worker *> workers;
};

} // namespace UV
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_UV_SERVER_H
