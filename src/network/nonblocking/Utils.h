#ifndef AFINA_NETWORK_NONBLOCKING_UTILS_H
#define AFINA_NETWORK_NONBLOCKING_UTILS_H

#include "ServerImpl.h"
#include <cstdint>

namespace Afina {
namespace Network {
namespace NonBlocking {
	
struct ep_fd {
    int fd;
    ep_fd(int fd_) : fd(fd_) {}
    virtual void advance(uint32_t) = 0;
    virtual ~ep_fd() {}
}; 

int setsocknonblocking(int sock);
void * Epoll_Proxy(void *p);
int epoll_modify(int epoll_fd, int how, uint32_t events, ep_fd &target);


} // namespace NonBlocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_NONBLOCKING_UTILS_H
