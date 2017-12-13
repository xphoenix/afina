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

static int setsocknonblocking(int sock) {
        int flags = fcntl(sock, F_GETFL, NULL);
        if (flags == -1)
            return flags;
        flags |= O_NONBLOCK;
        return fcntl(sock, F_SETFL, flags);
    }

} // namespace NonBlocking
} // namespace Network
} // namespace Afina
