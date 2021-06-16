#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value){
    if(key.size() + value.size() > _max_size){
        return false;
    }
    auto iter = _lru_index.find(key);
    if(iter != _lru_index.end()){
        _update(iter->second.get());
        std::string &val = iter->second.get().value;
        _free_space += val.size();
        while(_free_space < value.size()){
            _delete();
        }
        val = value;
        _free_space -= value.size();
        return true;
    }

    while(_free_space < key.size() + value.size()){
        _delete();
    }
    _free_space -= (key.size() + value.size());
    _add_elem(key, value);
    _lru_index.insert(std::make_pair(std::reference_wrapper<const std::string>(_lru_tail->key),
                      std::reference_wrapper<lru_node>(*_lru_tail)));
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value){
	if(key.size() + value.size() > _max_size){
        return false;
    }
    if(_lru_index.count(key)){
        return false;
    }
    while(_free_space < key.size() + value.size()){
        _delete();
    }
    _free_space -= (key.size() + value.size());
    _add_elem(key, value);
    _lru_index.insert(std::make_pair(std::reference_wrapper<const std::string>(_lru_tail->key),
                                     std::reference_wrapper<lru_node>(*_lru_tail)));
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value){
    if(value.size() > _max_size){
        return false;
    }
    auto iter = _lru_index.find(key);
    if(iter == _lru_index.end()){
        return false;
    }
    while(_free_space < value.size()){
        _delete();
    }
    _update(iter->second.get());
    std::string & _value = iter->second.get().value;
    _free_space += _value.size();
    _value = value;
    _free_space -= _value.size();
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key){
    if(!_lru_index.count(key)){
        return false;
    }
    auto iter = _lru_index.find(key);
    lru_node &node = iter->second;
    _lru_index.erase(std::reference_wrapper<const std::string>(key));
    if((!node.prev) && (!node.next)){
        _lru_head.reset();
    } else if(!node.prev){
        _lru_head = std::move(_lru_head->next);
    } else if(!node.next){
        _lru_tail = node.prev;
        _lru_tail->next.reset();
    } else{
        node.next->prev = node.prev;
        node.prev->next = std::move(node.next);
    }
    _free_space += node.key.size() + node.value.size();
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value){
    auto iter = _lru_index.find(key);
    if(iter == _lru_index.end()){
        return false;
    }
    _update(iter->second.get());
    value = iter -> second.get().value;
    return true;
}

void  SimpleLRU::_update(lru_node &node){
    if (_lru_tail->key != node.key){
        if(_lru_head->key != node.key){
            _lru_tail->next = std::move(node.prev->next);
            node.next->prev = node.prev;
            node.prev->next = std::move(node.next);
            _lru_tail = &node;
        } else{
            _lru_tail->next = std::move(_lru_head);
            _lru_head = std::move(node.next);
            node.prev = _lru_tail;
            _lru_tail = &node;
            _lru_head->prev = nullptr;
        }
    }
}

void SimpleLRU::_delete(){
    lru_node *lru = _lru_head.get();
    _free_space += (lru->key.size() + lru->value.size());
    _lru_index.erase(lru->key);
    if(_lru_head->next){
        _lru_head = std::move(_lru_head->next);
    } else{
        _lru_head.reset();
    }
}

void SimpleLRU::_add_elem(const std::string &key, const std::string &value){
    std::unique_ptr<lru_node> elem(new lru_node(key, value));
    if(!_lru_head){
        _lru_head = std::move(elem);
        _lru_tail = _lru_head.get();
    } else{
        _lru_tail->next = std::move(elem);
        _lru_tail->next->prev = _lru_tail;
        _lru_tail = _lru_tail->next.get();
    }
}

} // namespace Backend
} // namespace Afina
