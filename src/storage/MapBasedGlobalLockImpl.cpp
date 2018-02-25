#include "MapBasedGlobalLockImpl.h"

#include <mutex>

namespace Afina {
namespace Backend {

// LRUListNode
template <typename T>
void LRUListNode::Next(LRUListNode<T> *new_node) {
    _next = new_node;
    if (_next)
        _next->_prev = this;
}

void LRUListNode::Prev(LRUListNode *new_node) {
    _prev = new_node;
    if (_prev)
        _prev->_next = this;
}

void LRUListNode::SoftDelete() {
    if (_next)
        _next->_prev = _prev;

    if (_prev)
        _prev->_next = _next;
}

void LRUListNode::HardDelete() {
    SoftDelete();
    delete this;
}

LRUListNode* LRUListNode::getPrev() {
    return _prev;
}

LRUListNode* LRUListNode::getNext() {
    return _next;
}

template <typename T>
T LRUListNode::Value() {
    return _value;
}

template <typename T>
void LRUListNode::Value(const T& new_value) {
    _value = new_value;
}


// LRUList
template <typename T>
void LRUList::Append(const T& value) {
    LRUListNode<T> *new_node = new LRUListNode<T>(value);

    if (_tail)
        _tail->Next(new_node);
    else
        _tail = new_node;

    if (!_head)
        _head = _tail;
}

template <typename T>
void LRUList::Up(LRUListNode<T> *node_to_up) {
    node_to_up->SoftDelete();
    _head->Prev(node_to_up);
    _head = node_to_up;
}

template <typename T>
T LRUList::Head() {
    return _head;
}

template <typename T>
void LRUList::DeleteNode(LRUListNode<T>* node) {

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

    node->HardDelete();
}

void LRUList::DeleteHead() {
    auto tmp = _head->_next;
    _head->HardDelete();
    _head = tmp;
}

LRUList::~LRUList() {
    while (_head) {
        auto tmp = _head->_next();
        delete _head;
        _head = tmp;
    }
}

bool MapBasedGlobalLockImpl::DeleteLRU() {
    auto it = _lru->Head()->Value();
    if (it) {
        _backend.erase(it);
    }
    _lru->DeleteHead();
}

bool MapBasedGlobalLockImpl::Insert(const std::string &key, const std::string &value) {
    auto it = _backend.insert(std::make_pair( key, std::make_pair(value, _backend.end()) ));
    _lru->Append(it.first);
    auto ListNodePointer = it.first->second.value.second;
    ListNodePointer->Value(it.first);
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {

    // If end of size and elem doesn't exist
    if (_backend.size() == _max_size() &&
        _backend.find(key) == _backend.end())
            MapBasedGlobalLockImpl::DeleteLRU(); // Need delete last recently used

    // Insert new
    MapBasedGlobalLockImpl::Insert(key, value);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {

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

    if (_backend.find(key) != _backend.end()) {

        // Update key with value
        MapBasedGlobalLockImpl::Insert(key, value);
        // And up in the queue
        _lru->Up(_backend[key].value.second);
        return true;
    }

    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) {

    auto ListNodePointer = _backend[key].value.second;

    // Delete from map
    _backend.erase(key);
    // Delete from LRU list
    _lru.DeleteNode(ListNodePointer);
}
// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {

    if (_backend.find(key) == _backend.end())
        return false;

    value = _backend[key].value.first;
    // And up in queue
    _lru->Up(_backend[key].value.second);

    return true;
}

} // namespace Backend
} // namespace Afina
