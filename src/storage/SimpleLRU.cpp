#include "SimpleLRU.h"

#include <utility>
#include <iostream>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	if (key.size() + value.size() > _max_size) {
		return false;
	}

	while (key.size() + value.size()  + _cur_size > _max_size) {
        auto *node = new lru_node;
        std::swap(*node, *_lru_head);
        _lru_index.erase(node->key);
        delete_node(*node);
        delete node;
	}

	auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
	lru_node *new_node = add_node_to_tail(key, value);

	if (it != _lru_index.end()) {
        auto *node = new lru_node;
        std::swap(*node, it->second.get());
        _lru_index.at(std::reference_wrapper<const std::string>(key)) = std::reference_wrapper<lru_node>(*new_node);
        delete_node(*node);
        delete node;
	} else {
        if (_lru_head == nullptr) {
            _lru_head = std::unique_ptr<lru_node>(new_node);
        }
        _lru_index.insert({std::reference_wrapper<const std::string>(new_node->key),
                           std::reference_wrapper<lru_node>(*new_node)});
	}
	return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));

	if (it != _lru_index.end()) {
        return false;
    }
    if (key.size() + value.size() > _max_size) {
        return false;
    }

    while (key.size() + value.size()  + _cur_size > _max_size) {
        auto *node = new lru_node;
        std::swap(*node, *_lru_head);
        _lru_index.erase(node->key);
        delete_node(*node);
        delete node;
    }

    lru_node *new_node = add_node_to_tail(key, value);
    if (_lru_head == nullptr) {
        _lru_head = std::unique_ptr<lru_node>(new_node);
    }
    _lru_index.insert({std::reference_wrapper<const std::string>(new_node->key),
            std::reference_wrapper<lru_node>(*new_node)});
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (it == _lru_index.end()) {
        return false;
    }

    if (key.size() + value.size() > _max_size) {
        return false;
    }
    while (value.size() - it->second.get().value.size() + _cur_size > _max_size) {
        if (key != _lru_head->key) {
            auto *node = new lru_node;
            std::swap(*node, *_lru_head);
            _lru_index.erase(node->key);
            delete_node(*node);
            delete node;
        } else {
            delete_node(*_lru_head);
        }
    }

    lru_node *new_node = add_node_to_tail(key, value);
    auto *node = new lru_node;
    std::swap(*node, it->second.get());
    _lru_index.at(std::reference_wrapper<const std::string>(key)) = std::ref(*new_node);
    delete_node(*node);
    delete node;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (it == _lru_index.end()) {
        return false;
    }
    auto *node = new lru_node;
    std::swap(*node, it->second.get());
    _lru_index.erase(key);
    delete_node(*node);
    delete node;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (it == _lru_index.end()) {
        return false;
    }
    value = it->second.get().value;
    return true;
}

SimpleLRU::lru_node *SimpleLRU::add_node_to_tail(std::string key, std::string value) {
    _cur_size += key.size() + value.size();
    auto *new_node = new lru_node;
    new_node->key = std::move(key);
    new_node->value = std::move(value);
    new_node->prev = _lru_tail; // if list is empty then _lru_tail is nullptr
    if (_lru_tail != nullptr) {
        new_node->prev->next = std::unique_ptr<lru_node>(new_node);
    }
    new_node->next = nullptr;
    _lru_tail = new_node;
    return new_node;
}

void SimpleLRU::delete_node(SimpleLRU::lru_node &node) {
    _cur_size -= (node.key.size() + node.value.size());
    if (node.next == nullptr) {
        if (node.prev == nullptr) { // it's only _lru_head = _lru_tail
            _lru_head = nullptr;
            _lru_tail = nullptr;
        } else { // it's _lru_tail
            node.prev->next = nullptr;
            _lru_tail = node.prev;
        }
    } else {
        if (node.prev == nullptr) { // it's _lru_head
            _lru_head = std::move(node.next);
            _lru_head->prev = nullptr;
        } else { // inside the list
            node.prev->next = std::move(node.next);
            node.next->prev = node.prev;
        }
    }
}

} // namespace Backend
} // namespace Afina
