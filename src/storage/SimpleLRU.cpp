#include "SimpleLRU.h"

namespace Afina {
namespace Backend {




bool SimpleLRU::Set(const std::string &key, const std::string &value, lru_node &node) { 
	while (key.size() + value.size() + _curr_size > _max_size) {
		DeleteTail();
	}
	size_t old_size = node.value.size();
	node.value = value;
	if (node.next != nullptr) {
		node.next->prev = node.prev;
	}
	if (&node != _lru_head.get()) {
		auto tmp_node = std::move(node.prev->next);
		node.prev->next = std::move(node.next);
		if (tmp_node.get() != _lru_head->prev) {
			tmp_node->prev = _lru_head->prev;
		}
		tmp_node->next = std::move(_lru_head);
		tmp_node->next->prev = tmp_node.get();
		_lru_head = std::move(tmp_node);
	}
	else {
		_lru_head->value = value;

	}
	_curr_size += value.size() - old_size;
	return true;
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) { 
	if (key.size() + value.size() > _max_size)
		return false;
	auto elem = _lru_index.find(key);
	if (elem != _lru_index.end())
		return Set(key, value, elem->second);
	else
		return false;
}


void SimpleLRU::DeleteTail() {
	auto tail = _lru_head->prev;
	auto key = tail->key;
	_lru_index.erase(key);
	if (tail != _lru_head.get()) { // в списке больше одного элемента
		_lru_head->prev = tail->prev;
		tail->prev->next.reset();
	}
	else {
		_lru_head.reset();		
	}
}


void SimpleLRU::DeleteHead() {
	auto key = _lru_head->key;
	_lru_index.erase(key);
	if (_lru_head->prev != _lru_head.get()) { // в списке больше одного элемента
		auto tmp_head = std::move(_lru_head);
		_lru_head = std::move(tmp_head->next);
		_lru_head->prev = tmp_head->prev;
		tmp_head->next = nullptr;
		tmp_head.reset();
	}
	else {
		_lru_head.reset();		
	}
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) { 
 	auto elem = _lru_index.find(key);
	if (elem == _lru_index.end()) {
		return false;
	}
	lru_node &node = elem->second;
	_curr_size -= key.size() + node.value.size();
	if (&node == _lru_head->prev) {
		DeleteTail();
	}
	else if (&node == _lru_head.get()) {
		DeleteHead();
	}
	else {
		_lru_index.erase(elem);
		node.next->prev = node.prev;
		auto smart_node = std::move(node.prev->next); 
		node.prev->next = std::move(node.next);
		smart_node.reset();
	}
	return true; 
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) { 
	auto elem = _lru_index.find(key);
	if (elem == _lru_index.end()) {
		return false;
	}
	else {
		lru_node &node = elem->second;
		value = node.value;
		return true;
	}
}

bool SimpleLRU::PutNew(const std::string &key, const std::string &value) {
	while (key.size() + value.size() + _curr_size > _max_size) {
		Delete(_lru_head->prev->key);
	}
	if (_lru_head == nullptr) {
		_lru_head = std::move(std::unique_ptr<lru_node>(new lru_node(key, value)));
		_lru_head->prev = _lru_head.get();
	}
	else {
		std::unique_ptr<lru_node> _new_head (new lru_node(key, value));
		_new_head->next = std::move(_lru_head);
		// храню конец списка в prev головы для быстрого удаления
		_new_head->prev = _new_head->next->prev;
		_new_head->next->prev = _new_head.get();
		_lru_head = std::move(_new_head);
	}
	_lru_index.insert(std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>>(_lru_head->key, *_lru_head));
	_curr_size += key.size() + value.size();
	return true;
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	if (key.size() + value.size() > _max_size)
		return false;
	auto elem = _lru_index.find(key);
	if (elem != _lru_index.end()) {
		return Set(key, value, elem->second);
	}
	else {
		return PutNew(key, value);
	}
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
	if (key.size() + value.size() > _max_size)
		return false;
	if (_lru_index.find(key) != _lru_index.end())
		return false;
	else
		return PutNew(key, value);
}



} // namespace Backend
} // namespace Afina
