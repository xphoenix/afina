#include "MapBasedGlobalLockImpl.h"

#include <mutex>
#include <iostream>

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
    node_to_up->Delete();
    _head->Prev(node_to_up);
    _head = node_to_up;
}

template <typename T>
LRUListNode<T>& LRUList<T>::Head() {
    return *_head;
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
    
    auto it = _lru.Head().Value();
    _backend.erase(it);
    _lru.DeleteHead();

    return true;
}

bool MapBasedGlobalLockImpl::Insert(const std::string &key, const std::string &value) {

    Value map_value;
    map_value.value = std::make_pair(
        value, 
        new LRUListNode<map_iterator>(_backend.end())
    );

    auto it = _backend.insert(std::make_pair(
        key, 
        map_value
    ));
    _lru.Append(it.first);
    auto ListNodePointer = it.first->second.value.second;
    ListNodePointer->Value(it.first);

    return true;
}

bool MapBasedGlobalLockImpl::Update(const std::string &key, const std::string &value, map_iterator& elem_iter) {
    Value map_value;

    auto ListNodePointer = elem_iter->second.value.second;

    map_value.value = std::make_pair(
        value,
        ListNodePointer
    );

    _backend[key] = map_value;
    _lru.Up(ListNodePointer);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_global_lock);

    auto find_iter = _backend.find(key);
    bool exists = find_iter != _backend.end();

    // If end of size and elem doesn't exist
    if (_backend.size() == _max_size && !exists)
            MapBasedGlobalLockImpl::DeleteLRU(); // Need delete last recently used

    if (!exists)
        // Insert new
        return MapBasedGlobalLockImpl::Insert(key, value);
    else
        return MapBasedGlobalLockImpl::Update(key, value, find_iter);
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_global_lock);

    if (_backend.find(key) == _backend.end()) {

        if (_backend.size() == _max_size)
            MapBasedGlobalLockImpl::DeleteLRU();
        
        MapBasedGlobalLockImpl::Insert(key, value);
        return true;
    }

    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_global_lock);

    auto find_iter = _backend.find(key);
    bool exists = find_iter != _backend.end();

    if (!exists) {

        // Update key with value
        MapBasedGlobalLockImpl::Update(key, value, find_iter);
        // And up in the queue
        _lru.Up(_backend.find(key)->second.value.second);
        return true;
    }

    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) {
    std::lock_guard<std::mutex> lock(_global_lock);

    auto ListNodePointer = _backend[key].value.second;

    // Delete from map
    _backend.erase(key);
    // Delete from LRU list
    _lru.DeleteNode(ListNodePointer);
}
// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
    std::lock_guard<std::mutex> lock(_global_lock);

    if (_backend.find(key) == _backend.end())
        return false;

    auto find_iter = _backend.find(key);
    value = find_iter->second.value.first;
    // And up in queue
    auto ListNodePointer = _backend.find(key)->second.value.second;
    _lru.Up(ListNodePointer);

    return true;
}

} // namespace Backend
} // namespace Afina
