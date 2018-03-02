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

bool MapBasedGlobalLockImpl::DeleteLRU() {

    auto key = _lru.Head()->Value();
    auto it = _backend.find(key);

    auto key_pointer = &(it->first.get());
    auto value_pointer = &(it->second.first.get());

    _backend.erase(it);
    _lru.DeleteHead();

    delete key_pointer;
    delete value_pointer;

    return true;
}

bool MapBasedGlobalLockImpl::_Insert(const std::string &key, const std::string &value) {

    std::string* new_key_pointer   = new std::string(key);
    std::string* new_value_pointer = new std::string(value);

    _lru.Append(string_reference(*new_key_pointer));

    std::pair<string_reference, LRUListNode<string_reference>*> to_insert(
            string_reference(*new_value_pointer),
            _lru.Tail()
    );

    _backend.insert(std::make_pair(
            string_reference(*new_key_pointer),
            to_insert
    ));

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

    // Update value in unordered_map
    it->second.first = const_cast<std::string&>(value);
    _lru.Up(ListNodePointer);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {

    auto find_iter = _backend.find(const_cast<std::string&>(key));
    bool exists = find_iter != _backend.end();

    // If end of size and elem doesn't exist
    if (_backend.size() == _max_size && !exists)
        MapBasedGlobalLockImpl::DeleteLRU(); // Need delete last recently used

    if (!exists)
        // Insert new
        return MapBasedGlobalLockImpl::_Insert(key, value);
    else
        return MapBasedGlobalLockImpl::_Update(key, value, find_iter);
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {

    if (_backend.find(const_cast<std::string&>(key)) == _backend.end()) {

        if (_backend.size() == _max_size)
            MapBasedGlobalLockImpl::DeleteLRU();

        MapBasedGlobalLockImpl::_Insert(key, value);
        return true;
    }

    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {

    auto find_iter = _backend.find(const_cast<std::string&>(key));
    bool exists = find_iter != _backend.end();

    if (exists) {

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

    auto it = _backend.find(const_cast<std::string&>(key));
    if (it == _backend.end())
        return false;

    auto ListNodePointer = it->second.second;

    // Delete from map
    _backend.erase(const_cast<std::string&>(key));
    // Delete from LRU list
    _lru.DeleteNode(ListNodePointer);
}
// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {

    if (_backend.find(const_cast<std::string&>(key)) == _backend.end())
        return false;

    auto find_iter = _backend.find(const_cast<std::string&>(key));
    value = find_iter->second.first;

    // And up in queue
    auto ListNodePointer = _backend.find(const_cast<std::string&>(key))->second.second;
    _lru.Up(ListNodePointer);

    return true;
}

} // namespace Backend
} // namespace Afina
