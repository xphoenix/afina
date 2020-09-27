#include "SimpleLRU.h"

#include <iostream>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put( const std::string &key, const std::string &value ){ 

	push(key, value);
	_lru_index.insert({ key, std::ref(*_lru_head) });	

	return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent( const std::string &key, const std::string &value ){

	if( _lru_index.find(key) == _lru_index.end() ){
		
		push(key, value);
		_lru_index.insert({ key, std::ref(*_lru_head) });

		return true;
	}
       
	return false; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set( const std::string &key, const std::string &value ){

	if( _lru_index.find(key) != _lru_index.end() ){
		auto it = _lru_index.find(key);
		lru_node &node = it->second.get();
		node.value = value;
		return true;
	}	

	return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete( const std::string &key ){

	if( _lru_index.find(key) != _lru_index.end() ){
		
		auto it = _lru_index.find(key);
		pop( it->second.get() );
		_lru_index.erase(it);
		
		return true;
	}

	return false; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get( const std::string &key, std::string &value ){

	if( _lru_index.find(key) != _lru_index.end() ){
		auto it = _lru_index.find(key);
		auto& node = it->second.get();
		value = node.value;
		print(_lru_head);
		std::cout << "\n____\n";
                pop(node);
                push(node.key, node.value);
		return true;
	}
       
	return false; 
}

bool SimpleLRU::push( const std::string &key, const std::string &value ){

	if( _node_count ){
                if ( _lru_head )
		{
			std::cout << "jere1" << std::endl;
			auto* ptr = new lru_node{key, {}, {}, {}};//{key, value, std::move(_lru_head), nullptr};
			std::cout << "jere1.5" << std::endl;
			ptr->value = value;
			ptr->next = std::move(_lru_head);
			ptr->prev = nullptr;
			_lru_head.reset( ptr );
			std::cout << "jere2" << std::endl;
		} else
			_lru_head.reset( new lru_node{key, value, nullptr, nullptr} );
                // _lru_head = std::make_unique<lru_node>(lru_node{value, std::move(_lru_head), nullptr});
                if ( _lru_head->next )
			_lru_head->next->prev = _lru_head.get();
	}
	else{
		_lru_head.reset( new lru_node{key, value, nullptr, nullptr});

		// _lru_head = std::make_unique<lru_node>(lru_node{key, value, nullptr, nullptr});
		_lru_tail = _lru_head.get();
	}
                
	++_node_count;

	return true;
}

bool SimpleLRU::pop( lru_node& node ){

	if( !_node_count )
		return false;		

	if( node.next )
		if ( node.prev )
			node.next->prev = node.prev;
		else
			node.next->prev = nullptr;	
	if( node.prev )
		if ( node.next )
			node.prev->next = std::move(node.next);
		else
			node.prev->next = nullptr;
/*	
	if( _node_count > 1 ){

		tail = tail->prev;
		tail->next = nullptr;
	}
	else
	if( _node_count == 1 ){
		head = nullptr;
		tail = nullptr;
	}
	else
		throw std::runtime_error("pop(): No elements!");
*/
	--_node_count;
	print(_lru_head);
	return true;
}

void SimpleLRU::print(std::unique_ptr<lru_node>& node)
{
	if ( !node )
		return;
	std::cout << node->prev << ' ' << node.get() << ' ' << node->next.get() << std::endl;
	print(node->next);
}

} // namespace Backend
} // namespace Afina
