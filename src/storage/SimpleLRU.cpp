#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	auto it = _lru_index.find(key);
	if (it != _lru_index.end()) {
		pop(it->second.get());
	}

	if (!push(key, value))
		return false;

	return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) { 
	auto it = _lru_index.find(key);
	if (it == _lru_index.end()) {
		if (!push(key, value)) {
			return false;
		}
	}
	else {
		return false;
	}
	
	return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
	auto it = _lru_index.find(key);
	if (it == _lru_index.end()) {
		return false;
	}

	auto& search = it->second.get();
	pop(search);
	if (!push(key, value)) {
		push(key, search.value);
		return false;
	}
	return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
	auto it = _lru_index.find(key);
	if (it == _lru_index.end()) {
		return false;
	}

	auto& search = it->second.get();
	pop(search);

	return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
	auto it = _lru_index.find(key);
	if (it == _lru_index.end()) {
		return false;
	}

	auto& search = it->second.get();
	value = search.value;

	pop(search);
	push(key, value);
	return true;
}

bool SimpleLRU::push(const std::string &key, const std::string &value) {
	if (key.size() + value.size() > _max_size) {
		return false;
	}

	while (_lru_head && _curr_size + key.size() + value.size() > _max_size) {
		pop(*_lru_head.get());
	}

	auto* new_node = new lru_node{key, value, nullptr, std::move(_lru_head)};
	if (!new_node) {
		return false;
	}

	if (!_lru_head) {
		_lru_head.reset(new_node);
	}
	else {
		auto *tail = _lru_head.get();
		while (tail->next.get() != nullptr) {
			tail = tail->next.get();
		}
		tail->next.reset(new_node);
		new_node->prev.reset(tail);
	}

	_lru_index.insert({key, std::ref(*new_node)});
	_curr_size += (key.size() + value.size());

	return true;
}

bool SimpleLRU::pop(lru_node& node) {
	if (node.next) {
		if (node.prev) {
			node.next->prev = std::move(node.prev);
		}
		else {
			node.next->prev.reset();
		}
	}

	if (node.prev) {
		if (node.next) {
			node.prev->next = std::move(node.next);
		}
		else {
			node.prev->next.reset();
		}
	}

	_lru_index.erase(node.key);
	_curr_size -= node.key.size() + node.value.size();

	return true;
}

} // namespace Backend
} // namespace Afina
