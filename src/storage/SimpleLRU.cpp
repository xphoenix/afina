#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }

    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        resetValue(it->second.get(), value);
    }
    else {
        insertValue(key, value);
    }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }

    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) {
        return false;
    }

    insertValue(key, value);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }

    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    resetValue (it->second.get(), value);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    lru_node &node = it->second.get();
    _lru_index.erase(it);

    _remains += key.size() + node.value.size();

    if (node.prev == nullptr) {
        if (node.next == nullptr) {
            _lru_tail = nullptr;
            _lru_head.reset();
        }
        else {
            _lru_head.reset(node.next.release());
            _lru_head->prev = nullptr;
        }
    }
    else {
        if (node.next == nullptr) {
            _lru_tail = node.prev;
            _lru_tail->next.reset();
        }
        else {
            node.next->prev = node.prev;
            node.prev->next.reset(node.next.release());
        }
    }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    value = it->second.get().value;
    setHead(it->second.get());
    return true;
}


void SimpleLRU::insertValue(const std::string &key, const std::string &value) {
    _remains -= key.size() + value.size();
    while(_remains < 0) {
        dropTail();
    }

    if (_lru_tail == nullptr) {
        _lru_tail = new lru_node(key, value);
        _lru_head.reset(_lru_tail);
    }
    else {
        //auto newNode = std::make_unique<lru_node>(key, value);
        auto newNode = std::unique_ptr<lru_node>(new lru_node(key, value));
        _lru_head->prev = newNode.get();
        _lru_head.swap(newNode->next);
        _lru_head.swap(newNode);
    }

    _lru_index.insert(std::make_pair(
        std::ref(_lru_head->key),
        std::ref(*_lru_head)
    ));
}


void SimpleLRU::resetValue(lru_node &node, const std::string &value) {
    setHead(node);

    while(_remains < 0) {
        dropTail();
    }
    node.value = value;
}


void SimpleLRU::setHead(lru_node &node) {
    if (node.prev == nullptr) return;

    _lru_head->prev = &node;
    if (node.next == nullptr) {
        _lru_tail = node.prev;
    }
    else {
        node.next->prev = node.prev;
    }
    node.prev->next.swap(node.next);
    node.next.swap(_lru_head);
    _lru_head->prev = nullptr;
}


void SimpleLRU::dropTail() {
    _remains += _lru_tail->key.size() + _lru_tail->value.size();

    _lru_index.erase(_lru_tail->key);

    if (_lru_tail->prev == nullptr) {
        _lru_tail = nullptr;
        _lru_head.reset();
    }
    else {
        _lru_tail = _lru_tail->prev;
        _lru_tail->next.reset();
    }
}

} // namespace Backend
} // namespace Afina
