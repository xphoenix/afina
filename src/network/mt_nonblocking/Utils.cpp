#include "Utils.h"

#include <stdexcept>

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace Afina {
namespace Network {
namespace MTnonblock {

void make_socket_non_blocking(int sfd) {
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("Failed to call fcntl to get socket flags");
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        throw std::runtime_error("Failed to call fcntl to set socket flags");
    }
}

} // namespace MTnonblock
} // namespace Network
} // namespace Afina
