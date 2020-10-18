#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

SimpleLRU::~SimpleLRU() {
    _lru_index.clear();
     while (_lru_tail) {
         _lru_tail->next.reset();
         _lru_tail = _lru_tail->prev;
     }
     _lru_head.reset();
 }
 
bool SimpleLRU::add_node(const std::string key, const std::string value) {
    std::unique_ptr<lru_node> temp(new lru_node);
    temp->size = key.length() + value.length();
    if (temp->size > _max_size) {return false;} 
    temp->next = NULL;                  
    temp->key = key;                    
    temp->value = value;  
    fit_list(temp->size); 
    
    if (_lru_head != NULL) {
        temp->prev = _lru_tail; 
        _lru_tail->next = std::move(temp);               
        _lru_tail = _lru_tail->next.get();                     
    } 
    else {
        temp->prev = NULL;               
        _lru_head = std::move(temp); 
        _lru_tail = _lru_head.get();    
    }
    return true;
}

 void SimpleLRU::fit_list(size_t itemSize) {
    _cur_size += itemSize;
    while (_cur_size > _max_size) {
        if (_lru_head.get() == _lru_tail) {
            _cur_size -= _lru_head->size;
            _lru_index.erase(_lru_head->key);
            _lru_head.reset();
            _lru_tail = NULL;
            return;
        }

        _cur_size -= _lru_head->size;
        std::unique_ptr<lru_node> temp(new lru_node);
        temp = std::move(_lru_head->next); 
        temp->prev = NULL;
        _lru_head = std::move(temp); 
    }
}

void SimpleLRU::delete_node(lru_node *delNode){
    _cur_size -= delNode->size; 
    _lru_index.erase(delNode->key);

    auto prvNode = delNode->prev;
    auto nxtNode = delNode->next.get();
    
    if (delNode != _lru_head.get()){
        prvNode->next = std::move(delNode->next);
    } else {
        _lru_head = std::move(delNode->next);
    }

    if (delNode != _lru_tail){
        nxtNode->prev = delNode->prev;
    } else {
        _lru_tail = delNode->prev;
    }
}
 
 bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (!PutIfAbsent(key, value)) {
        return Set(key, value);
    }
    return true;
}

bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if ( _lru_index.find(key) != _lru_index.end() ) {return false;}
    if (!add_node(key, value)) {return false;} 
    _lru_index.insert(std::make_pair(std::reference_wrapper<const std::string>(_lru_tail->key),
                      std::reference_wrapper<lru_node>(*_lru_tail)));
    return true;
}

bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if ( it == _lru_index.end() ) {return false;}
    lru_node& curNode = it->second;
    size_t newSize = key.length() + value.length();     
    
    _cur_size += newSize - curNode.size;
    curNode.size = newSize;
    curNode.value = value; 

    // _lru_index.erase(key);
    return true;
}

bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if ( it == _lru_index.end() ) {return false;}
    lru_node& curNode = it->second;
    value = curNode.value;
    return true;
}

bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if ( it == _lru_index.end() ) {return false;}
    lru_node& delNode = it->second;

    delete_node(&delNode);
    return true;
}

} // namespace Backend
} // namespace Afina
