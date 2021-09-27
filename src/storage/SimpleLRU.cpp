#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

std::size_t SimpleLRU::get_size () {
    return _size;
}


bool SimpleLRU::is_overflow(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size)
        return true;
    return false;
}

void SimpleLRU::add_key_value(const std::string &key, const std::string &value) {
    while (_size + key.size() + value.size() > _max_size) {
        erase_last();
    }
    auto *node = new lru_node{key, value, nullptr, nullptr};
    if (_lru_head) {
        _lru_head->prev = node;
    } else {
        _lru_tail = node;
    }
    node->next = std::move(_lru_head);
    _lru_head.reset(node);
    _lru_index.insert({std::reference_wrapper<const std::string>(node->key), std::reference_wrapper<lru_node>(*node)});
    _size += key.size() + value.size();
}


void SimpleLRU::erase_last() {
    _size -= _lru_tail->key.size() + _lru_tail->value.size();
    _lru_index.erase(_lru_tail->key);
    if (_lru_head.get() != _lru_tail) {
        _lru_tail = _lru_tail->prev;
        _lru_tail->next.reset(nullptr);
    } else {
        _lru_head.reset(nullptr);
    }
}

void SimpleLRU::update_the_position(lru_node &node) {
    lru_node *node_ptr = &node;
    if (_lru_head.get() == node_ptr) {
        return;
    }
    if (node_ptr->next) {
        std::unique_ptr<lru_node> temp = std::move(_lru_head);
        _lru_head = std::move(node_ptr->prev->next);
        node_ptr->prev->next = std::move(node_ptr->next);
        temp->prev = node_ptr;
        node_ptr->next = std::move(temp);
        node_ptr->prev->next->prev = node_ptr->prev;
        node_ptr->prev = nullptr;
    } else {
        _lru_tail = node_ptr->prev;
        _lru_head->prev = node_ptr;
        node_ptr->next = std::move(_lru_head);
        _lru_head = std::move(node_ptr->prev->next);
    }
}

void SimpleLRU::set_existed(lru_node &node, const std::string &value) {
    update_the_position(node);
    if (value.size() > node.value.size()) {
        while (_size + value.size() - node.value.size() > _max_size) {
            erase_last();
        }
    }
    _size += value.size() - node.value.size();
    node.value = value;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (is_overflow(key, value)) {
        return false;
    }
    auto node = _lru_index.find(key);
    if (node != _lru_index.end()) {
        set_existed((node->second).get(), value);
    } else {
        add_key_value(key, value);
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (is_overflow(key, value)) {
        return false;
    }
    if (_lru_index.find(key) == _lru_index.end()) {
        add_key_value(key, value);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (is_overflow(key, value)) {
        return false;
    }
    auto node = _lru_index.find(key);
    if (node != _lru_index.end()) {
        set_existed((node->second).get(), value);
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    if (_lru_index.find(key) == _lru_index.end()) {
        return false;
    }
    lru_node *node_ptr = &(_lru_index.find(key)->second.get());
    _lru_index.erase(_lru_index.find(key));

    if (node_ptr != _lru_head.get()) {
        if (node_ptr->next) {
            node_ptr->next->prev = node_ptr->prev;
            node_ptr->prev->next = std::move(node_ptr->next);
        } else {
            _lru_tail = node_ptr->prev;
            node_ptr->prev->next.reset();
        }
    } else {
        if (node_ptr->next) {
            node_ptr->next->prev = nullptr;
            _lru_head = std::move(node_ptr->next);
        } else {
            _lru_head.reset();
            _lru_tail = nullptr;
        }
    }
    _size -= node_ptr->key.size() + node_ptr->value.size();
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto node = _lru_index.find(key);
    if (node == _lru_index.end()) {
        return false;
    }
    update_the_position(node->second.get());
    value = _lru_head->value;
    return true;
}

} // namespace Backend
} // namespace Afina
