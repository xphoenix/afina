#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// Try to free size bytes and not delete pass_node
bool SimpleLRU::_FreeSpace(std::size_t size, lru_node* pass_node) {
    if (size > _max_size)
        return false;
    lru_node* buf = _lru_tail;
    lru_node* prev_node = nullptr;
    while (size > _max_size - _cur_size) {
        if (buf == pass_node) {
            buf = buf->prev;
        }
        else if (buf) {
            if (buf == _lru_tail) // if tail elem
                _lru_tail = buf->prev;
            // update index
            _lru_index.erase(buf->key);
            // update size
            _cur_size -= buf->key.size() + buf->value.size();
            // update list
            prev_node = buf->prev;
            if (!prev_node) // if last elem
                _lru_head = std::move(buf->next);
            else
                prev_node->next = std::move(buf->next);
            // step
            buf = prev_node;
        }
        else
            return false;
    }
    return true;
}

// Move mode to head
void SimpleLRU::_MoveToHead(lru_node* node) {
    if (node == _lru_head.get()) // if head
        return;
    if (node == _lru_tail) // if tail
        _lru_tail = _lru_tail->prev;

    // delete
    lru_node *prev_node = node->prev;
    auto moving_node = std::move(prev_node->next);
    prev_node->next = std::move(moving_node->next);
    // set to head
    _lru_head->prev = node;
    moving_node->next =std::move(_lru_head);
    _lru_head = std::move(moving_node);
}

// Add new node to head
void SimpleLRU::_AddHead(const std::string &key, const std::string &value) {
    // add to list
    lru_node* node = new lru_node{key, value, nullptr, std::move(_lru_head)};
    _lru_head = std::unique_ptr<lru_node>(node);
    if (_lru_tail) // if not first elem
        node->next.get()->prev = node;
    else
        _lru_tail = node;
    // add to index
    _lru_index.emplace(std::make_pair(std::ref(node->key), std::ref(*node)));
    _cur_size += (key.size() + value.size());
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    auto index_node = _lru_index.find(std::ref(key));
    if (index_node != _lru_index.end()) { // if key already exists
        size_t cur_value_size = index_node->second.get().value.size();
        if (_FreeSpace(value.size()-cur_value_size, &index_node->second.get())) {
            index_node->second.get().value = value;
            _cur_size -= cur_value_size - value.size();
            _MoveToHead(&index_node->second.get());
            return true;
        }
        return false;
    }
    else {
        if (_FreeSpace(key.size() + value.size())) {
            _AddHead(key, value);
            return true;
        }
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    auto index_node = _lru_index.find(std::ref(key));
    if (index_node == _lru_index.end()) {
        if (this->_FreeSpace(key.size() + value.size())) {
            // replace head
            _AddHead(key, value);
            // add to index
            _lru_index.emplace(std::ref(key), std::ref(*(_lru_head.get())));
            return true;
        }
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto index_node = _lru_index.find(std::ref(key));
    if (index_node != _lru_index.end()) { // if key  exists
        size_t cur_value_size = index_node->second.get().value.size();
        if (_FreeSpace(value.size()-cur_value_size, &index_node->second.get())) {
            index_node->second.get().value = value;
            _cur_size -= cur_value_size - value.size();
            _MoveToHead(&index_node->second.get());
            return true;
        }
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto index_node = _lru_index.find(key);
    if (index_node == _lru_index.end()) {
        return false;
    }
    lru_node* cur_node = &index_node->second.get();
    _lru_index.erase(index_node);

    _MoveToHead(cur_node);

    _lru_head = std::move(cur_node->next);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto index_node = _lru_index.find(std::ref(key));
    // if key already exists
    if (index_node == _lru_index.end())
        return false;
    value = index_node->second.get().value;
    return true;
}

} // namespace Backend
} // namespace Afina
