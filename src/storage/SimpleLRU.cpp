#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {

}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {

}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    lru_node &erased = it->second.get();
    _lru_index.erase(it);

    if (erased.prev == nullptr) {
        if (erased.next == nullptr) {
            _lru_head.reset();
        }
        else {
            _lru_head.reset(erased.next.release());
            _lru_head->prev = nullptr;
        }
    }
    else {
        if (erased.next == nullptr) {
            erased.prev->next.reset();
        }
        else {
            erased.next->prev = erased.prev;
            erased.prev->next.reset(erased.next.release());
        }
    }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    if(_lru_head == nullptr) {
        return false;
    }

    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    value = it->second.get().value;
    setHead(it->second.get());
    return true;
}


void SimpleLRU::setHead(lru_node &node) {
    if (node.prev == nullptr) return;

    _lru_head->prev = &node;
    if (node.next != nullptr) {
        node.next->prev = node.prev;
    }
    node.prev->next.swap(node.next);
    node.next.swap(_lru_head);
    _lru_head->prev = nullptr;
}

} // namespace Backend
} // namespace Afina
