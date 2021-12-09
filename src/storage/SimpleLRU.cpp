#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto in_cache = _lru_index.find(key);
    if (in_cache == _lru_index.end()) {
        _put_absent(key, value);
        return true;
    } else {
        lru_node &node = in_cache->second.get();
        _set_existing(node, value);
        return true;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto in_cache = _lru_index.find(key);
    if (in_cache == _lru_index.end()) {
        _put_absent(key, value);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto in_cache = _lru_index.find(key);
    if (in_cache != _lru_index.end()) {
        lru_node &node = in_cache->second.get();
        _set_existing(node, value);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto in_cache = _lru_index.find(key);
    if (in_cache != _lru_index.end()) {
        lru_node &node = in_cache->second.get();
        _cache_size -= node.key.size() + node.value.size();
        _lru_index.erase(key);
        std::swap(node.prev, node.next->prev);
        std::swap(node.next, node.next->prev->next);
        node.next.reset();
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto in_cache = _lru_index.find(key);
    if (in_cache != _lru_index.end()) {
        value = in_cache->second.get().value;
        lru_node &node = in_cache->second.get();
        std::swap(node.prev, node.next->prev);
        std::swap(node.next, node.next->prev->next);
        std::swap(node.prev, _lru_head->next->prev);
        std::swap(node.next, _lru_head->next);
        return true;
    }
    return false;
}

void SimpleLRU::_put_absent(const std::string &key, const std::string &value) {
    while (_max_size - _cache_size < key.size() + value.size()) {
        _delete_least_recent();
    }
    _cache_size += key.size() + value.size();
    auto node = new lru_node{key, value, nullptr, nullptr};
    node->prev = node;
    node->next.reset(node);
    std::swap(node->prev, _lru_head->next->prev);
    std::swap(node->next, _lru_head->next);
    _lru_index.emplace(std::reference_wrapper<const std::string>(node->key), std::reference_wrapper<lru_node>(*node));
}
void SimpleLRU::_set_existing(SimpleLRU::lru_node &node, const std::string &value) {
    std::swap(node.prev, node.next->prev);
    std::swap(node.next, node.next->prev->next);
    std::swap(node.prev, _lru_head->next->prev);
    std::swap(node.next, _lru_head->next);
    _cache_size -= node.value.size();
    node.value = "";
    while (_max_size - _cache_size < value.size()) {
        _delete_least_recent();
    }
    _cache_size += value.size();
    node.value = value;
}

void SimpleLRU::_delete_least_recent() {
    auto node = _lru_head->prev;
    std::swap(node->prev, node->next->prev);
    std::swap(node->next, node->next->prev->next);
    _cache_size -= node->key.size() + node->value.size();
    _lru_index.erase(node->key);
    node->next.reset();
}

} // namespace Backend
} // namespace Afina