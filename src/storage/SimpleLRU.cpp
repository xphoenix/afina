#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    mapT::iterator it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return _insert_kv(key, value);
    } else {
        return _update_kv(it, value);
        // return _update_kv(key, value);
    }
}
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    mapT::iterator it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return _insert_kv(key, value);
    } else {
        return false;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    mapT::iterator it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    } else {
        return _update_kv(it, value);
        // return _update_kv(key, value);
    }
}
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    mapT::iterator it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    } else {
        return remove_node(it->second.get());
    }
}
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    mapT::iterator it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    value = it->second.get().value;
    return _move_to_tail(it->second.get());
}

bool SimpleLRU::_insert_kv(const std::string &key, const std::string &value) {
    std::size_t size = key.size() + value.size();
    if (size > _max_size) {
        return false;
    }
    while (size > _free_size) {
        remove_node(*_lru_head);
    }
    lru_node *new_node(new lru_node(key, value));
    _lru_index.insert(std::make_pair(std::reference_wrapper<const std::string>(new_node->key),
                                     std::reference_wrapper<lru_node>(*new_node)));
    _free_size -= size;
    return _insert(*new_node);
}

bool SimpleLRU::remove_node(lru_node &delete_node) {
    _lru_index.erase(delete_node.key);
    _free_size += ((_lru_tail->key).size() + (_lru_tail->value).size());
    std::unique_ptr<lru_node> tmp;
    if (&delete_node == _lru_head.get()) {
        tmp.swap(_lru_head);
        _lru_head.swap(tmp->next);
        _lru_head->prev = nullptr;
        return true;
    } else if (&delete_node == _lru_tail) {
        tmp.swap(_lru_tail->prev->next);
        _lru_tail = tmp->prev;
        _lru_tail->next.reset(nullptr);
        return true;
    }
    tmp.swap(delete_node.prev->next);
    tmp->prev->next.swap(tmp->next);
    tmp->next->prev = tmp->prev;
    return true;
}

bool SimpleLRU::_update_kv(const mapT::iterator &it, const std::string &value) {
    // bool SimpleLRU::_update_kv(const std::string &key, const std::string &value){
    lru_node &old_node = it->second.get();
    size_t size = old_node.key.size() + value.size();
    if (size > _max_size) {
        return false;
    }
    // auto it = _lru_index.find(key);
    // std::size_t old_size = it->second.get().key.size() + it->second.get().value.size();
    std::size_t old_size = old_node.key.size() + old_node.value.size();
    _move_to_tail(old_node);
    //_move_to_tail(it->second.get());
    while (size > _free_size + old_size) {
        remove_node(*_lru_head);
    }
    old_node.value = value;
    // it->second.get().value = value;
    _free_size += old_size - size;
    return true;
}

// Insertion a node - create prts
bool SimpleLRU::_insert(lru_node &node) {
    if (_lru_tail == nullptr) {
        node.next == nullptr;
        node.prev == nullptr;
        _lru_head.reset(&node);
        _lru_tail = &node;
    } else {
        node.next = nullptr;
        node.prev = _lru_tail;
        _lru_tail->next.reset(&node);
        _lru_tail = &node;
    }
    return true;
}
// move the element to the tail
bool SimpleLRU::_move_to_tail(lru_node &node) {
    if (node.next == nullptr) // at end
        return true;
    else if (node.prev == nullptr) { // at begin
        node.next->prev = node.prev;
        node.next.swap(_lru_head);
        node.next.swap(_lru_tail->next);
        node.prev = _lru_tail;
        _lru_tail = &node;
        return true;
    } else {
        node.next->prev = node.prev;
        node.prev->next.swap(node.next);
        node.next.swap(_lru_tail->next);
        node.prev = _lru_tail;
        _lru_tail = &node;
        return true;
    }
    return false;
}

} // namespace Backend
} // namespace Afina
