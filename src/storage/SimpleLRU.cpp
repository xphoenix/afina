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

	if (it != _lru_index.end()) {
        _cur_size += value.size() - it->second.get().value.size();
        it->second.get().value = value;
        it->second.get().key = key;
        move_to_tail(it);
	} else {
        lru_node *new_node = add_node_to_tail(key, value);
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
        delete_oldest_node(); // TODO if elem with key deleted
    }
    _cur_size += value.size() - it->second.get().value.size();
    it->second.get().value = value;
    it->second.get().key = key;
    move_to_tail(it);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (it == _lru_index.end()) {
        return false;
    }

    lru_node &del_node = it->second.get();
    _lru_index.erase(key);
    swap(del_node.prev->next, del_node.next);
    del_node.next->prev = del_node.prev;
    del_node.next = nullptr;
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
    auto *new_node = new lru_node {std::move(key), std::move(value), _lru_tail->prev, nullptr};
    new_node->next = std::unique_ptr<lru_node>(new_node);
    std::swap(new_node->next, _lru_tail->prev->next);
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

void SimpleLRU::move_to_tail(
        std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<std::string>>::iterator &it) {
    it->second.get().next->prev = it->second.get().prev;
    swap(it->second.get().prev->next, it->second.get().next);
    it->second.get().prev = _lru_tail->prev;
    swap(it->second.get().next, _lru_tail->prev->next);
    _lru_tail->prev = &it->second.get();
}


} // namespace Backend
} // namespace Afina
