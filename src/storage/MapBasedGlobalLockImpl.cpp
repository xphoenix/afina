#include "MapBasedGlobalLockImpl.h"
 
#include <mutex>
#include <iostream>
#include <algorithm>
#include <functional>


namespace Afina {
namespace Backend {

// LRUListNode
template <typename T>
void LRUListNode<T>::Next(LRUListNode<T> *new_node) {
    if (new_node != this) {
        _next = new_node;

        if (_next)
            _next->_prev = this;
    }
}

template <typename T>
void LRUListNode<T>::Prev(LRUListNode<T> *new_node) {
    if (new_node != this) {
        _prev = new_node;

        if (_prev)
            _prev->_next = this;
    }

}

template <typename T>
void LRUListNode<T>::Delete() {
    if (_next)
        _next->_prev = _prev;

    if (_prev)
        _prev->_next = _next;

    _next = nullptr;
    _prev = nullptr;
}

template <typename T>
LRUListNode<T>* LRUListNode<T>::getPrev() {
    return _prev;
}

template <typename T>
LRUListNode<T>* LRUListNode<T>::getNext() {
    return _next;
}

template <typename T>
T LRUListNode<T>::Value() {
    return _value;
}

template <typename T>
void LRUListNode<T>::Value(const T& new_value) {
    _value = new_value;
}

template <typename T>
LRUListNode<T>::~LRUListNode() {}


// LRUList
template <typename T>
void LRUList<T>::Append(const T& value) {
    LRUListNode<T> *new_node = new LRUListNode<T>(value);
    if (_tail)
        _tail->Next(new_node);

    _tail = new_node;

    if (!_head)
        _head = _tail;
}

template <typename T>
void LRUList<T>::Up(LRUListNode<T> *node_to_up) {
    if (_tail != _head)
        return;

    node_to_up->Delete();
    _tail->Next(node_to_up);
    _tail = node_to_up;
}

template <typename T>
LRUListNode<T>* LRUList<T>::Head() {
    return _head;
}

template <typename T>
LRUListNode<T>* LRUList<T>::Tail() {
    return _tail;
}

template <typename T>
void LRUList<T>::DeleteNode(LRUListNode<T>* node) {

    // Need update head and tail of list if
    // node is one of them
    if (node == _head || node == _tail) {
        if (node == _head && node == _tail) {
            _head = _tail = nullptr;
        } else {
            if (node == _head) {
                _head = _head->getNext();
            } else {
                _tail = _tail->getPrev();
            }
        }
    }

    node->Delete();
    delete node;
}

template <typename T>
void LRUList<T>::DeleteHead() {
    auto tmp = _head->getNext();
    _head->Delete();
    delete _head;
    _head = tmp;
}

template <typename T>
LRUList<T>::~LRUList() {
    while (_head) {
        auto tmp = _head->getNext();
        delete _head;
        _head = tmp;
    }
}

bool MapBasedGlobalLockImpl::_DeleteLRU() {

    if (!_lru.Head()) {
        std::cerr << "Try to delete null LRU Head" << std::endl;
        return false;
    }

    auto key = _lru.Head()->Value();
    auto it  = _backend.find(key);

    auto key_pointer   = &(it->first.get());
    auto value_pointer = it->second.first;

    _backend.erase(it);
    _lru.DeleteHead();

    _usage_size -= key_pointer->size() + value_pointer->size();

    delete key_pointer;
    delete value_pointer;

    return true;
}

bool MapBasedGlobalLockImpl::_Insert(const std::string &key, const std::string &value) {

    std::string* new_key_pointer   = new std::string(key);
    std::string* new_value_pointer = new std::string(value);

    _lru.Append(string_reference(*new_key_pointer));

    std::pair<string_pointer, LRUListNode<string_reference>*> to_insert(
            new_value_pointer,
            _lru.Tail()
    );

    _backend.insert(std::make_pair(
            string_reference(*new_key_pointer),
            to_insert
    ));

    // Update usage size
    _usage_size += key.size() + value.size();

    return true;
}

bool MapBasedGlobalLockImpl::_Update(const std::string &key,
                                     const std::string &value,
                                     unordered_map_type::iterator &it) {

    auto ListNodePointer = it->second.second;

    std::pair<string_reference, LRUListNode<string_reference>*> to_update(
            string_reference(const_cast<std::string&>(value)),
            ListNodePointer
    );

    // Sub last value size
    _usage_size -= it->second.first->size();

    // Update value in unordered_map
    *it->second.first = value;
    _lru.Up(ListNodePointer);

    // Update usage size
    _usage_size += value.size();

    return true;
}

bool MapBasedGlobalLockImpl::_ClearUsageData(const std::string &new_key,
                    const std::string &new_value,
                    unordered_map_type::iterator &find_iter) {

    if (new_key.size() + new_value.size() > _max_size)
        return false;

    bool exists = find_iter != _backend.end();

    if (!exists) {
        while (_usage_size + new_key.size() + new_value.size() > _max_size) {
            if (!MapBasedGlobalLockImpl::_DeleteLRU())
                return false;
        }
    } else {
        auto last_value_size = find_iter->second.first->size();
        while (_usage_size - last_value_size + new_value.size() > _max_size) {
            if (!MapBasedGlobalLockImpl::_DeleteLRU())
                return false;
        }
    }

    return true;
}

// See MapBasedGlobalLockImpl.hc
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_global_lock);

    auto find_iter = _backend.find(const_cast<std::string&>(key));
    bool exists = find_iter != _backend.end();

    // Delete data if need
    if (!MapBasedGlobalLockImpl::_ClearUsageData(key, value, find_iter))
        return false;

    if (!exists)
        // Insert new
        return MapBasedGlobalLockImpl::_Insert(key, value);
    else
        return MapBasedGlobalLockImpl::_Update(key, value, find_iter);
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_global_lock);

    auto find_iter = _backend.find(const_cast<std::string&>(key));

    if (find_iter == _backend.end()) {

        // Delete data if need
        if (!MapBasedGlobalLockImpl::_ClearUsageData(key, value, find_iter))
            return false;

        MapBasedGlobalLockImpl::_Insert(key, value);
        return true;
    }

    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_global_lock);

    auto find_iter = _backend.find(const_cast<std::string&>(key));
    bool exists = find_iter != _backend.end();

    if (exists) {

        // Delete data if need
        if (!MapBasedGlobalLockImpl::_ClearUsageData(key, value, find_iter))
            return false;

        // Update key with value
        MapBasedGlobalLockImpl::_Update(key, value, find_iter);
        // And up in the queue
        _lru.Up(_backend.find(const_cast<std::string&>(key))->second.second);
        return true;
    }

    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) {
    std::lock_guard<std::mutex> lock(_global_lock);

    auto it = _backend.find(const_cast<std::string&>(key));
    if (it == _backend.end())
        return false;

    auto ListNodePointer = it->second.second;

    // Delete from map
    _backend.erase(const_cast<std::string&>(key));
    // Delete from LRU list
    _lru.DeleteNode(ListNodePointer);

    auto value_size = it->second.first->size();
    _usage_size -= key.size() + value_size;

    return true;
}
// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
    std::lock_guard<std::mutex> lock(_global_lock);

    if (_backend.find(const_cast<std::string&>(key)) == _backend.end())
        return false;

    auto find_iter = _backend.find(const_cast<std::string&>(key));
    value = *find_iter->second.first;

    // And up in queue
    auto ListNodePointer = _backend.find(const_cast<std::string&>(key))->second.second;
    _lru.Up(ListNodePointer);

    return true;
}

MapBasedGlobalLockImpl::~MapBasedGlobalLockImpl() {
    // Remember, that we allocated memory for key and value
    for (auto &it : _backend) {
        auto key_pointer   = &(it.first.get());
        auto value_pointer = it.second.first;

        delete key_pointer;
        delete value_pointer;
    }
}

} // namespace Backend
} // namespace Afina
