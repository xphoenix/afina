#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutNew(const std::string &key, const std::string &value) {
    return false;
}

bool SimpleLRU::PutOld(const std::string &key, const std::string &value) {
    return false;
}

bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size)
        return false;
    if(_lru_index.find(key) != _lru_index.end())
        return PutOld(key, value);
    return PutNew(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size)
        return false;
    if (_lru_index.find(key)!=_lru_index.end())
        return false;
    return PutNew(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size)
        return false;
    if (_lru_index.find(key) == _lru_index.end())
        return false
    return PutOld(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) const {
    return false;
}

} // namespace Backend
} // namespace Afina
