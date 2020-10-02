#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) { 
if ((key.size() + value.size()) > _max_size){
        return false;
    }
    map_type::iterator it = _lru_index.find(key);
    if (it != _lru_index.end()){
        return update_node(it, value);
    }
    else{
        return put_node(key, value);
    }
 }

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if ((key.size() + value.size()) > _max_size){
        return false;
    }
map_type::iterator it=_lru_index.find(key);
    if (it == _lru_index.end()){
        return put_node(key, value);
    }
    else{
        return false;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if ((key.size() + value.size()) > _max_size){
        return false;
    }
    map_type::iterator it = _lru_index.find(key);
    if (it != _lru_index.end()){
        return update_node(it, value);
    }
    else{
        return false;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    map_type::iterator it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    return remove_node(it->second.get());
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    map_type::iterator it = _lru_index.find(key);
    if (it == _lru_index.end()){
        return false;
    }
    value = it->second.get().value;
    return move_node_tail(it->second.get());
}

bool SimpleLRU::put_node(const std::string &key, const std::string &value) {
    while (_current_size + key.size() + value.size() > _max_size){
        remove_node(*_lru_head);
    }
    _current_size += key.size() + value.size();
    auto new_node = std::make_unique<lru_node>(key, value);
    if (_lru_tail != nullptr) {
        new_node->prev = _lru_tail;
        _lru_tail->next.swap(new_node);
        _lru_tail = _lru_tail->next.get();

    } else {
        _lru_tail = new_node.get();
        _lru_head.swap(new_node);
    }

    _lru_index.insert(std::make_pair(std::reference_wrapper< const std::string>(_lru_tail->key), std::reference_wrapper<lru_node> (*_lru_tail)));
    return true;
}

bool SimpleLRU::update_node(const map_type::iterator &it, const std::string &new_value) {
    lru_node &old_node = it->second.get();
    if (move_node_tail(old_node)) {
        while (_current_size - old_node.value.size() + new_value.size() > _max_size){
                remove_node(*_lru_head);
        }
        if (old_node.key.empty()){
             throw std::runtime_error("Division by zero!");
        }
        _current_size += new_value.size() - old_node.value.size();
        old_node.value = new_value;
        return true;
    } else
        return false;
}

bool SimpleLRU::move_node_tail(lru_node &_node) {
    if (&_node == _lru_tail){
        return true;
    }
    if (&_node == _lru_head.get()) {
        _lru_head.swap(_node.next);
        _lru_head->prev = nullptr;
    } else {
        _node.next->prev = _node.prev;
        _node.prev->next.swap(_node.next);
    }
    _lru_tail->next.swap(_node.next);
    _node.prev = _lru_tail;
    _lru_tail = &_node;
    return true;
}

bool SimpleLRU::remove_node(lru_node &delete_node) {
    _lru_index.erase(delete_node.key);
    _current_size -= ((_lru_tail->key).size() + (_lru_tail->value).size());
    std::unique_ptr<lru_node> tmp;
    if (&delete_node == _lru_head.get()) {
        tmp.swap(_lru_head);
        _lru_head.swap(tmp->next);
        _lru_head->prev = nullptr;
        return true;
    } else if (&delete_node == _lru_tail) {
        tmp.swap(_lru_tail->prev->next);
        _lru_tail = tmp->prev;
        _lru_tail->next.reset(nullptr);
        return true;
    }
    tmp.swap(delete_node.prev->next);
    tmp->prev->next.swap(tmp->next);
    tmp->next->prev = tmp->prev;
    return true;
}
} // namespace Backend
} // namespace Afina
