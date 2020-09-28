#include "SimpleLRU.h"


namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put( const std::string &key, const std::string &value ){ 
	auto it = _lru_index.find(key);
	if ( it != _lru_index.end() ){
		pop(it->second.get());
	}
	if (!push(key, value))
		return false;
	return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent( const std::string &key, const std::string &value ){

	if( _lru_index.find(key) == _lru_index.end() ){
		
		push(key, value);

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
                pop(node);
                push(key, value);
		return true;
	}
       
	return false; 
}

bool SimpleLRU::push( const std::string &key, const std::string &value ){

	while ( _lru_tail && _cur_size + (key.size() + value.size()) > _max_size )
		pop(*_lru_tail);
	
	lru_node* ptr = new lru_node{key, value, nullptr, nullptr};
	if( !ptr )	
		return false;
	
	if( _node_count ){

                if( _lru_head ){

			ptr->next = std::move(_lru_head);
			ptr->prev = nullptr;
			_lru_head.reset( ptr );
		}
		else
			_lru_head.reset(ptr);
                
		if( _lru_head->next ){

			_lru_head->next->prev = _lru_head.get();
		}
	}
	else{
		_lru_head.reset(ptr);
		_lru_tail = _lru_head.get();
	}
	if( _lru_head )
	{
        	_lru_index.insert({ key, std::ref(*_lru_head) });
		++_node_count;
		_cur_size += (key.size() + value.size());
		return true;
	}
	return true;
}

bool SimpleLRU::pop( lru_node& node ){

	_lru_index.erase(node.key);

	if( !_node_count )
		return false;
	
	if( node.next )
	{
		if ( node.prev )
			node.next->prev = node.prev;
		else
			node.next->prev = nullptr;	
	}
	else{

		_lru_tail = _lru_tail->prev;

	}

	if( node.prev )
	{
		if ( node.next )
			node.prev->next = std::move(node.next);
		else
		{
			node.prev->next = nullptr;
		}
	}
	
	--_node_count;
	_cur_size -= (node.key.size() + node.value.size());

	return true;
}

} // namespace Backend
} // namespace Afina
