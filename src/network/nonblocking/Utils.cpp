#include "Utils.h"
#include <stdexcept>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include <iostream>

namespace Afina {
namespace Network {
namespace NonBlocking {

int setsocknonblocking(int sock) {
        int flags = fcntl(sock, F_GETFL, NULL);
        if (flags == -1)
            return flags;
        flags |= O_NONBLOCK;
        return fcntl(sock, F_SETFL, flags);
    }

void * Epoll_Proxy(void *p){
    ServerImpl *srv = reinterpret_cast<ServerImpl *>(p);
    srv->RunEpoll();
    return 0;
} 

int epoll_modify(int epoll_fd, int how, uint32_t events, ep_fd &target) {
    struct epoll_event new_ev {  // создаем структуры для epoll_ctl
            events, { (void *)&target }
    };
    return epoll_ctl(epoll_fd, how, target.fd, &new_ev);
}
 



} // namespace NonBlocking
} // namespace Network
} // namespace Afina
