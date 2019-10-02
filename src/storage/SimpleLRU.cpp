#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {

void SimpleLRU::Erase(lru_node& node) {

    _free_size += node.key.size() + node.value.size();

    if (node.prev == nullptr) {
        _lru_head.swap(node.next);
        node.next.release();
        return;
    }

    if (node.next == nullptr) {
        _lru_tail = node.prev;
        node.prev->next.release();
        return;
     }

    node.next->prev = node.prev;
    node.prev->next.swap(node.next);
    node.next.release();
    node.prev = nullptr;

}

void SimpleLRU::PopBack() {

    _lru_index.erase(_lru_tail->key);
    _lru_tail = _lru_tail->prev;
    _lru_tail->next.reset();
    _free_size += _lru_tail->key.size() + _lru_tail->value.size();

}

void SimpleLRU::InsertToHead(lru_node* node) {

    while (_free_size < (node->key.size() + node->value.size()))
        PopBack();

    _free_size -= node->key.size() + node->value.size();
    if (_lru_head != nullptr) {
        auto old_head = _lru_head.release();
        old_head->prev = node;
        node->prev = nullptr;
        node->next.reset(old_head);
    } else {
        _lru_tail = node;
    }

    _lru_head.reset(node);


}
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {

    if ((key.size() + value.size()) >= _max_size)
        return false;

    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        it->second.get().value = value;
        Erase(it->second.get());
        InsertToHead(&it->second.get());
    } else {
        lru_node* node = new lru_node({key, value});
        InsertToHead(node);
        _lru_index.insert(std::make_pair(std::ref(node->key), std::ref(*node)));

    }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {

    auto it = _lru_index.find(key);
    if (it == _lru_index.end() && (key.size() + value.size()) < _max_size) {
        lru_node* node = new lru_node({key, value});
        InsertToHead(node);
        _lru_index.insert(std::make_pair(std::ref(node->key), std::ref(*node)));
        return true;

    }

    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if (it != _lru_index.end() && (key.size() + value.size()) < _max_size) {
        it->second.get().value = value;
        Erase(it->second.get());
        InsertToHead(&it->second.get());
        return true;

    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        Erase(it->second.get());
        delete &it->second.get();
        _lru_index.erase(it);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) { return false; }

    value = it->second.get().value;
    Erase(it->second.get());

    InsertToHead(&it->second.get());
    return true;

}


} // namespace Backend
} // namespace Afina
