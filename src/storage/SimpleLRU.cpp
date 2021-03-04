#include "SimpleLRU.h"
#include "iostream"

namespace Afina {
namespace Backend {

void SimpleLRU::LRU_delete() {
	lru_node* deleted_node = _lru_tail;
	_curr_size -= deleted_node->key.size() + deleted_node->value.size();
	_lru_index.erase(deleted_node->key);
	if (deleted_node != _lru_head.get())
	{
		_lru_tail = deleted_node->prev;
		_lru_tail->next.reset();
	}
	else //there is only one element in list
	{
		_lru_head.reset();
	}
}

void SimpleLRU::LRU_move(lru_node* moved_node) {
	if (_lru_head.get() == moved_node) //node is already a head
	{
		return;
	}
	if (!moved_node->next.get()) //move the last node
	{
		_lru_tail = moved_node->prev;
		_lru_head.get()->prev = moved_node;
		moved_node->next = std::move(_lru_head);
		_lru_head = std::move(moved_node->prev->next);
	}
	else
	{
		auto p = std::move(_lru_head);
		_lru_head = std::move(moved_node->prev->next);
		moved_node->prev->next = std::move(moved_node->next);
		p.get()->prev = moved_node;
		moved_node->next = std::move(p);
		moved_node->prev->next->prev = moved_node->prev;
	}
}

void SimpleLRU::PutIn(const std::string &key, const std::string &value, std::size_t node_size) {
	while (node_size + _curr_size > _max_size)
	{
		LRU_delete();
	}
	lru_node *new_node = new lru_node{key, value, nullptr, nullptr};
	//insert
	if (_lru_head.get())
	{
		_lru_head.get()->prev = new_node;
	}
	else
	{
		_lru_tail = new_node;
	}
	new_node->prev = nullptr;
	new_node->next = std::move(_lru_head);
	_lru_head.reset(new_node);
	//end of insert
	_lru_index.insert({std::reference_wrapper<const std::string>(new_node->key), std::reference_wrapper<lru_node>(*new_node)});
	_curr_size += node_size;
}

void SimpleLRU::SetIn(lru_node &node, const std::string &value) {
	int diff_size = value.size() - node.value.size(); //change size_t on int
	LRU_move(&node);
	while (_curr_size + diff_size > _max_size)
	{
		LRU_delete();
	}
	node.value = value;
	_curr_size += diff_size;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	std::size_t node_size = key.size() + value.size();
	if (node_size > _max_size) // can't guarantee invariant
	{
		return false;
	}
	auto node = _lru_index.find(key);
	
	if (node ==  _lru_index.end()) // there is no key
	{
		PutIn(key, value, node_size);
	}
	else
	{
		SetIn((node->second).get(), value);
	}
	return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) { 
	std::size_t node_size = key.size() + value.size();
	if (node_size > _max_size) // can't guarantee invariant
	{
		return false;
	}
	auto node = _lru_index.find(key);
	if (node ==  _lru_index.end()) // there is no key
	{
		PutIn(key, value, node_size);
		return true;
	}
	return false; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) { 
	int node_size = key.size() + value.size();
	if (node_size > _max_size) // can't guarantee invariant
	{
		return false;
	}
	auto node = _lru_index.find(key);
	if (node !=  _lru_index.end()) // key exists
	{
		SetIn((node->second).get(), value);
		return true; 
	}
	return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) { 
	auto p = _lru_index.find(key);
	if (p == _lru_index.end())
	{
		return false;
	}
	lru_node* node = &(p->second.get());
	_lru_index.erase(p);
	_curr_size -= node->key.size() + node->value.size();
	if (node == _lru_head.get()) //deleting head
	{
		if (!node->next.get()) //one element in list
		{
			_lru_tail = nullptr;
			_lru_head.reset();
		}
		else
		{
			node->next.get()->prev = nullptr;
			_lru_head = std::move(node->next);
		}
	}
	else if (!node->next.get()) //deleting last element
	{
		_lru_tail = node->prev;
		node->prev->next.reset();
	}
	else
	{
		node->next.get()->prev = node->prev;
		node->prev->next = std::move(node->next);
	}
	return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
	auto node = _lru_index.find(key);
	if (node !=  _lru_index.end())
	{
		value = node->second.get().value;
		LRU_move(&node->second.get());
		return true;
	}
	return false; 
}

} // namespace Backend
} // namespace Afina
