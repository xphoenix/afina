#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) { 
	
	if (key.size() + value.size() >= _max_size){
		return false; 
	}

	auto curr_pair = _lru_index.find(key);

	if (curr_pair == _lru_index.end()) {
		status = _Add(key, value);
	} else {
		lru_node* curr_node = &curr_pair->second.get();
		status = _Set(curr_node, value);
	}
	return status;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) { 
	
	if ((key.size() + value.size() >= _max_size) | (_lru_index.find(key) != _lru_index.end())) {
		return false;
	}	      
	
	status = _Add(key, value);
	
	return status;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
	
	if (key.size() + value.size() >= _max_size) {
		return false;
	}

	auto curr_pair = _lru_index.find(key);
	if (curr_pair == _lru_index.end()){
		return false;
	}

	lru_node* curr_node = &curr_pair->second.get();

	status = _Set(curr_node, value);

	return status;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
	
	auto curr_pair = _lru_index.find(key);
	
	if (curr_pair == _lru_index.end()) {
		return false;
	}
	
	lru_node* curr_node = &curr_pair->second.get();
	
	if (curr_node->prev) {
		if (curr_node->next) {
			curr_node->prev->next = std::move(curr_node->next);
		} else {
			_lru_tail = _lru_tail->prev;
		}
	} else {
		if (_lru_head->next) {
			_lru_head = std::move(_lru_head->next);
			_lru_head->prev = nullptr;
		} else {
			_lru_tail = nullptr;
			_lru_head.reset();
		}
	}

	_lru_index.erase(curr_pair);
	_curr_size -= key.size() + curr_node->value.size();
	return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {

	auto curr_pair = _lru_index.find(key);
	
	if (curr_pair == _lru_index.end()) {
                return false;
        }
	
	lru_node* curr_node = &curr_pair->second.get();
	value = curr_node->value;
	_move_to_head(curr_node);

	return true;
}


bool SimpleLRU::_Add(const std::string &key, const std::string &value) {
	
	lru_node* new_node = new lru_node{key, value};
	
	_lru_index.emplace(std::reference_wrapper<const std::string>(new_node->key), 
			std::reference_wrapper<lru_node>(*new_node));

	if (_lru_head == nullptr) {
		_lru_head = std::unique_ptr<lru_node>(new_node);
		_lru_tail = new_node;
		return true;
	}

	_lru_head->prev = new_node;
	new_node->next = std::move(_lru_head);
	_lru_head.reset(new_node);

	_clean_memory(key.size() + value.size());

	return true;
}

bool SimpleLRU::_Set(lru_node* node, const std::string &value) {
	
	node->value = value;
        _move_to_head(node);
	_clean_memory(node->key.size() + value.size());
	
	return true;
}

void SimpleLRU::_move_to_head(lru_node* node) {
	if (node == _lru_head.get()) { return; }

	if (node == _lru_tail) {
		_lru_tail = node->prev;
	} else {
		node->next->prev = node->prev;
	}
	std::unique_ptr<lru_node> curr_ptr = std::move(node->prev->next);	
	node->prev->next = std::move(node->next);
                
	_lru_head->prev = node;
	node->prev = nullptr;
        node->next = std::move(_lru_head);
        _lru_head = std::move(curr_ptr);
	
	return;
}

void SimpleLRU::_clean_memory(std::size_t pair_size) {
	_curr_size += pair_size;	
	if (_curr_size < _max_size) { return; }

	if (_lru_tail == _lru_head.get()) {
		_curr_size = 0;
		_lru_index.erase(_lru_head->key);
		_lru_head.reset();
		return;
	}
	
	while ((_curr_size >= _max_size) && _lru_head) {

		_curr_size -= _lru_tail->key.size() + _lru_tail->value.size();

		_lru_index.erase(_lru_tail->key);

		_lru_tail = _lru_tail->prev;
		_lru_tail->next.reset();
	}
	
	return;
}


} // namespace Backend
} // namespace Afina
