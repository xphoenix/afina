#include "SimpleLRU.h"
#include <algorithm>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto key_node = _lru_index.find(key);
    if (key_node != _lru_index.end()) {
        ReleaseSpace(std::max((size_t)0, value.size() - key_node->second.get().value.size()));
        key_node->second.get().value = value;
        MoveToTail(&key_node->second.get());
    } else {
        ReleaseSpace(key.size() + value.size());
        AddToTail(key, value);
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto key_node = _lru_index.find(key);
    if (key_node != _lru_index.end()) {
        return false;
    } else {
        ReleaseSpace(key.size() + value.size());
        AddToTail(key, value);
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto key_node = _lru_index.find(key);
    if (key_node != _lru_index.end()) {
        ReleaseSpace(value.size() - key_node->second.get().value.size());
        key_node->second.get().value = value;
        MoveToTail(&key_node->second.get());
    } else {
        return false;
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto key_node = _lru_index.find(key);
    if (key_node != _lru_index.end()) {
        RemoveNode(&key_node->second.get());
    } else {
        return false;
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto key_node = _lru_index.find(key);
    if (key_node != _lru_index.end()) {
        value = key_node->second.get().value;
        MoveToTail(&key_node->second.get());
    } else {
        return false;
    }
    return true;
}

void SimpleLRU::ReleaseSpace(size_t needed_free_size) {
    while (needed_free_size > _cur_free_size) {
        RemoveNode(_lru_head.get());
    }
}

void SimpleLRU::MoveToTail(lru_node *node) {
    if (_lru_tail == node) {
        return;
    }
    auto p_prev = node->prev;

    std::unique_ptr<lru_node> temp;
    if (p_prev != nullptr) {
        temp = std::move(p_prev->next);
        p_prev->next = std::move(temp->next);
        p_prev->next->prev = p_prev;
    } else {
        temp = std::move(_lru_head);
        _lru_head = std::move(temp->next);
        _lru_head->prev = nullptr;
    }
    _lru_tail->next = std::move(temp);
    _lru_tail->next->prev = _lru_tail;
    _lru_tail = _lru_tail->next.get();
    _lru_tail->next = nullptr;
}

void SimpleLRU::AddToTail(const std::string &key, const std::string &value) {
    std::unique_ptr<lru_node> temp(new lru_node(key, value));
    _cur_free_size = _cur_free_size - (key.size() + value.size());
    if (_lru_tail) {
        _lru_tail->next = std::move(temp);
        _lru_tail->next->prev = _lru_tail;
        _lru_tail = _lru_tail->next.get();
    } else {
        _lru_head = std::move(temp);
        _lru_tail = _lru_head.get();
    }
    _lru_index.emplace(std::ref(_lru_tail->key), std::ref(*_lru_tail));
}

void SimpleLRU::RemoveNode(lru_node *node) {
    auto p_next = node->next.get();
    auto p_prev = node->prev;
    _cur_free_size += node->key.size() + node->value.size();
    _lru_index.erase(node->key);

    if (p_next != nullptr) {
        if (p_prev != nullptr) {
            p_next->prev = p_prev;
            p_prev->next = std::move(node->next);
        } else {
            _lru_head = std::move(node->next);
            _lru_head->prev = nullptr;
        }
    } else {
        if (p_prev != nullptr) {
            _lru_tail = p_prev;
            p_prev->next = nullptr;
        } else {
            _lru_head = nullptr;
            _lru_tail = nullptr;
        }
    }
}

} // namespace Backend
} // namespace Afina
