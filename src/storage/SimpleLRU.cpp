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
        delete_oldest_node();
	}

	auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
	lru_node *new_node = add_node_to_tail(key, value);

	if (it != _lru_index.end()) {
        _lru_index.at(std::reference_wrapper<const std::string>(new_node->key)) = std::reference_wrapper<lru_node>(*new_node);
	} else {
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
        delete_oldest_node();
    }

    lru_node *new_node = add_node_to_tail(key, value);
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
        delete_oldest_node(); // if elem with key deleted
    }

    lru_node *new_node = add_node_to_tail(key, value);
    _lru_index.at(std::reference_wrapper<const std::string>(new_node->key)) = std::ref(*new_node);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (it == _lru_index.end()) {
        return false;
    }

    swap(it->second.get().prev->next, it->second.get().next);
    std::swap(it->second.get().next->prev, it->second.get().prev);
    //it->second.get().next = nullptr;
    _lru_index.at(std::reference_wrapper<const std::string>(key));
    _lru_index.erase(key);
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
    new_node->prev = _lru_tail->prev;
    new_node->next = std::unique_ptr<lru_node>(new_node);
    std::swap(new_node->next, new_node->prev->next);
    _lru_tail->prev = new_node;
    return new_node;
}

void SimpleLRU::delete_oldest_node() {
    lru_node *old_node = _lru_head->next.get();
    if (old_node == nullptr) {
        return;
    }
    _cur_size -= old_node->key.size() + old_node->value.size();
    _lru_index.erase(old_node->key);
    old_node->next->prev = _lru_head.get();
    swap(_lru_head->next, old_node->next);
    old_node->next = nullptr;
}

} // namespace Backend
} // namespace Afina
