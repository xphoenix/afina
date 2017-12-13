#ifndef AFINA_NETWORK_NONBLOCKING_UTILS_H
#define AFINA_NETWORK_NONBLOCKING_UTILS_H

#include "ServerImpl.h"

namespace Afina {
namespace Network {
namespace NonBlocking {

static int setsocknonblocking(int sock);

} // namespace NonBlocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_NONBLOCKING_UTILS_H
