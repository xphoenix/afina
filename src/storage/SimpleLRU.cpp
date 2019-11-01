#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {

void SimpleLRU::ChangeTheNode(lru_node& node, const std::string &value) {

    if (node.prev == nullptr) {
        _free_size += node.value.size() - value.size();
        node.value = value;
        while (_free_size < 0) {
            PopBack();
        }

        return;
    }
    _free_size += node.key.size() + node.value.size();
    node.value = value;

    if (node.next == nullptr) {
        _lru_tail = node.prev;
        node.prev->next.release();
        InsertToHead(&node);
        return;
     }

    node.next->prev = node.prev;
    node.prev->next.swap(node.next);
    node.next.release();
    node.prev = nullptr;

    InsertToHead(&node);

}

void SimpleLRU::PopBack() {
    _free_size += _lru_tail->key.size() + _lru_tail->value.size();
    if (_lru_tail == _lru_head.get()) {
        _lru_index.erase(_lru_tail->key);
        _lru_head.reset();
        _lru_tail = nullptr;

        return;
    }
    _lru_index.erase(_lru_tail->key);
    _lru_tail = _lru_tail->prev;
    _lru_tail->next.reset();


}

void SimpleLRU::InsertToHead(lru_node* node) {


    while (_free_size < (node->key.size() + node->value.size())) {
        PopBack();
    }

    _free_size -= node->key.size() + node->value.size();
    if (_lru_head != nullptr) {
        auto old_head = _lru_head.release();
        old_head->prev = node;
        node->prev = nullptr;
        node->next.reset(old_head);
    } else {

        _lru_tail = node;
        node->prev = nullptr;
        node->next = nullptr;
    }

    _lru_head.reset(node);



}
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {

    if ((key.size() + value.size()) >= _max_size) {
        return false;
    }

    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        ChangeTheNode(it->second.get(), value);
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
    if (it == _lru_index.end()) {
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
    if (it != _lru_index.end()) {
        ChangeTheNode(it->second.get(), value);
        return true;

    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        lru_node& node = it->second.get();
        _free_size += node.key.size() + node.value.size();
        _lru_index.erase(it);

        if (node.prev == nullptr) {
            _lru_head.swap(node.next);
            node.next.reset();
            return true;
        }

        if (node.next == nullptr) {
            _lru_tail = node.prev;
            node.prev->next.reset();
            return true;
         }

        node.next->prev = node.prev;
        node.prev->next.swap(node.next);
        node.next.reset();
        node.prev = nullptr;
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) { return false; }

    value = it->second.get().value;
    ChangeTheNode(it->second.get(), value);
    return true;

}


} // namespace Backend
} // namespace Afina
