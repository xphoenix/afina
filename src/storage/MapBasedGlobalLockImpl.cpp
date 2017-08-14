#include "MapBasedGlobalLockImpl.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
const std::string &MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) { return ""; }

// See MapBasedGlobalLockImpl.h
const std::string &MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) { return ""; }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) { return false; }

} // namespace MapBasedGlobalLockImpl
} // namespace Afina
