#include "SimpleLRU.h"

namespace Afina {
namespace Backend {
    
void SimpleLRU::_PutWithoutCheck(const std::string& key,
        const std::string& value) {
    while (_current_size + key.size() + value.size() > _max_size) {
        Delete(_lru_head->key);
    }
    
    auto new_node = std::make_unique<lru_node>();
    new_node->key = key;
    new_node->value = value;
    
    _lru_index.insert(std::make_pair(
        std::ref(new_node->key),
        std::ref(*new_node)
    ));
    _current_size += key.size() + value.size();
    
     if (!_lru_head) {
        new_node->prev = new_node.get();
        new_node->next.reset();
        _lru_head = std::move(new_node); 
    }
    else {
         new_node->prev = _lru_head->prev;
         new_node->next.reset();
         _lru_head->prev->next = std::move(new_node);
         _lru_head->prev = _lru_head->prev->next.get();
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) { 
        return false;
    }
    
    if (Set(key, value)) {}
    else {
        _PutWithoutCheck(key, value);
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) { 
        return false;
    }
    auto new_key = key;
    if (_lru_index.find(std::ref(new_key)) == _lru_index.end()) {
        _PutWithoutCheck(key, value);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) { 
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    
    if (Delete(key)) {
        _PutWithoutCheck(key, value);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto new_key = key;
    auto iter = _lru_index.find(std::ref(new_key));
    if (iter != _lru_index.end()) {
        auto& node = iter->second.get();
        _lru_index.erase(iter);
        _current_size -= node.key.size() + node.value.size();
        
        if (node.prev == &node) {
            // only head exists
            _lru_head.reset();
        }
        else {
            if (&node == _lru_head.get()) {
                // deleting head
                node.next->prev = node.prev;
                _lru_head = std::move(node.next);
            }
            else if (!node.next) {
                // deleting tail
                _lru_head->prev = node.prev;
                node.prev->next.reset();
            }
            else {
                node.next->prev = node.prev;
                node.prev->next = std::move(node.next);
            }
        }
        
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto new_key = key;
    auto iter = _lru_index.find(std::ref(new_key));
    if (iter == _lru_index.end()) {
        return false;
    }
    else {
        value = iter->second.get().value;
        return true;
    }
}

} // namespace Backend
} // namespace Afina
