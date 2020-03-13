#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    // std::cout << "Hello  " << key << "  " << value << "\n";
    auto it = _lru_index.find(std::ref(key));

    if (it == _lru_index.end()) {
        if (!shrink_size(key.size() + value.size()))
            return false;
        lru_node *new_node = add_node(key, value);
        if (new_node == nullptr)
            return false;
        _lru_index.insert(std::make_pair(std::ref(new_node->key), std::ref(*new_node)));
    } else {
        _cur_size -= it->second.get().value.size() + it->second.get().key.size();
        if (!(shrink_size(value.size() + it->second.get().key.size()))) {
            _cur_size += it->second.get().value.size() + it->second.get().key.size();
            return false;
        }
        move_to_head(it->second);
        _cur_size += value.size() + it->second.get().key.size();
        it->second.get().value = value;
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(std::ref(key));

    if (it != _lru_index.end())
        return false;
    if (!shrink_size(key.size() + value.size()))
        return false;
    lru_node *new_node = add_node(key, value);
    _lru_index.insert(std::make_pair(std::ref(new_node->key), std::ref(*new_node)));
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end())
        return false;
    if ((value.size() > it->second.get().value.size()) && (!shrink_size(value.size() - it->second.get().value.size())))
        return false;
    it->second.get().value = value;
    move_to_head(it->second);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end())
        return false;
    remove_node(it->second);
    _lru_index.erase(it);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    // std::cout << "Hello  " << key << "  " << "\n";
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end())
        return false;
    value = it->second.get().value;
    move_to_head(it->second);
    return true;
}

// PRIVATES
// Checks if possible to add node and delete LRU nodes if possible
bool SimpleLRU::shrink_size(std::size_t elem_size) {
    if (elem_size > _max_size)
        return false;

    while (elem_size + _cur_size > _max_size) {
        _cur_size -= _lru_tail->prev->key.size() + _lru_tail->prev->value.size();
        auto it = _lru_index.find(std::ref(_lru_tail->prev->key));
        _lru_index.erase(it);
        _lru_tail->prev = _lru_tail->prev->prev;
        _lru_tail->prev->next = std::move(_lru_tail->prev->next->next);
    }
    return true;
};

// Adds node
SimpleLRU::lru_node *SimpleLRU::add_node(const std::string &key, const std::string &value) {
    std::unique_ptr<lru_node> cur_node = std::unique_ptr<lru_node>(new lru_node(key, value));
    _cur_size += cur_node->key.size() + cur_node->value.size();
    cur_node->prev = _lru_head.get();
    cur_node->next = std::move(_lru_head->next);
    cur_node->next->prev = cur_node.get();
    _lru_head->next = std::move(cur_node);
    return _lru_head->next.get();
};

// Deletes node
void SimpleLRU::remove_node(std::reference_wrapper<SimpleLRU::lru_node> node) {
    _cur_size -= node.get().value.size() + node.get().key.size();
    node.get().next->prev = node.get().prev;
    node.get().prev->next = std::move(node.get().next);
};

// Moves node to head
void SimpleLRU::move_to_head(std::reference_wrapper<SimpleLRU::lru_node> node) {
    std::unique_ptr<lru_node> cur_node;
    cur_node = std::move(node.get().prev->next);
    node.get().next->prev = node.get().prev;
    node.get().prev->next = std::move(node.get().next);
    cur_node->next = std::move(_lru_head->next);
    cur_node->prev = _lru_head.get();
    cur_node->next->prev = cur_node.get();
    _lru_head->next = std::move(cur_node);
};

} // namespace Backend
} // namespace Afina
