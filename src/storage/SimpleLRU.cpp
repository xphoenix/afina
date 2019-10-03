#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()){
        return Set(key, value);
    }
    else {
        return PutIfAbsent(key, value);
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()){
        return false;
    }
    if (key.size() + value.size() > _max_size) {
        return false;
    }

    // if size of new node is too big
        // delete nodes from tail until enough space
        // and refresh _cur_size
    // create new node
    // insert it in head
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        // ? and that is all ? anything changes ?
        return false;
    }
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    size_t new_size = key.size() + value.size();
    size_t old_size = it->second.get().key.size() + it->second.get().value.size();

    // TODO: move founded node to head

    // if size of new node is too big
    if (_cur_size - old_size + new_size > _max_size) {
        // TODO: delete nodes from tail until enough space
        // and refresh _cur_size
    }

    // TODO: it->second.get() or changed link?
    it->second.get().value = value;

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    // TODO: delete node
    // and refresh _cur_size
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    value = it->second.get().value;
    return true;
}

} // namespace Backend
} // namespace Afina
