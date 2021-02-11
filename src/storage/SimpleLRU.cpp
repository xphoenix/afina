#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if(key.size() + value.size() > _max_size)
        return false;
    if (_lru_head == nullptr){
        InsertNodeToEmptyList(key, value);
    }
    else {
        auto map_elem = _lru_index.find(key);
        if (map_elem != _lru_index.end()) {
            while(value.size() >= _capacity + map_elem->second.get().value.size()) {
                DeleteWorstNode();
            }
            ChangeNodeValue(map_elem->second.get(), value);
            return true;
        }
        while(key.size() + value.size() >= _capacity + _lru_head->key.size() + _lru_head->value.size()) {
            DeleteWorstNode();
        }
        InsertNode(key, value);
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) { 
    if(key.size() + value.size() > _max_size)
        return false;
    if (_lru_head == nullptr){
        InsertNodeToEmptyList(key, value);
    }
    else {
        auto map_elem = _lru_index.find(key);
        if (map_elem != _lru_index.end())
            return false;
        while(key.size() + value.size() >= _capacity + _lru_head->key.size() + _lru_head->value.size()) {
            DeleteWorstNode();
        }
        InsertNode(key, value);
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if(key.size() + value.size() > _max_size)
        return false;
    auto map_elem = _lru_index.find(key);
    if(map_elem == _lru_index.end())
        return false;
    while(value.size() >= _capacity + map_elem->second.get().value.size()) {
        DeleteWorstNode();
    }
    ChangeNodeValue(map_elem->second.get(), value);
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) { 
    auto map_elem = _lru_index.find(key);
    if (map_elem == _lru_index.end())
        return false;
    lru_node& node = map_elem->second.get();
    if (node.prev == nullptr)
        DeleteWorstNode();
    else if(node.next == nullptr) {
        _capacity += key.size() + node.value.size();
        _lru_index.erase(key);
        _lru_tail = node.prev;
        _lru_tail->next.reset(nullptr);
    }
    else {
        _capacity += key.size() + node.value.size();
        _lru_index.erase(key);
        node.next->prev = node.prev;
        node.prev->next = std::move(node.next);
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto map_elem = _lru_index.find(key);
    if (map_elem == _lru_index.end())
        return false;
    lru_node& node = map_elem->second.get(); 
    value = node.value;
    if(node.next == nullptr)
        return true;
    else if(node.prev == nullptr) {
        node.next->prev = nullptr;
        node.prev = _lru_tail;
        std::unique_ptr<lru_node> tmp_ptr = std::move(_lru_head);
        _lru_head = std::move(node.next);
        _lru_tail->next = std::move(tmp_ptr);
        _lru_tail = &node;
        node.next = nullptr;
    }
    else {
        node.next->prev = node.prev;
        std::unique_ptr<lru_node> tmp_ptr = std::move(node.next);
        _lru_tail->next = std::move(node.prev->next);
        node.prev->next = std::move(tmp_ptr);
        node.prev = _lru_tail;
        node.next = nullptr;
        _lru_tail = &node;
    }
    return true; 
}

void SimpleLRU::DeleteWorstNode() {
    _capacity += _lru_head->key.size() + _lru_head->value.size();
    _lru_index.erase(_lru_head->key);
    if (_lru_head->next == nullptr) {
        _lru_head = std::move(_lru_head->next);
        _lru_tail = nullptr;
    }
    else {
        _lru_head = std::move(_lru_head->next);
        _lru_head->prev = nullptr;
    }
}

void SimpleLRU::InsertNodeToEmptyList(const std::string &key, const std::string &value) {
    lru_node* new_node = new lru_node{key, value, nullptr, nullptr};
    std::reference_wrapper<lru_node> wrapped_node(*new_node);
    std::reference_wrapper<const std::string> tmp_key(new_node->key);
    _lru_head = std::unique_ptr<lru_node>(new_node);
    _lru_index.insert(std::make_pair(tmp_key, wrapped_node));
    _capacity -= key.size() + value.size();
    _lru_tail = new_node;
}

void SimpleLRU::InsertNode(const std::string &key, const std::string &value) {
    lru_node* new_node = new lru_node{key, value, nullptr, nullptr};
    std::reference_wrapper<lru_node> wrapped_node(*new_node);
    std::reference_wrapper<const std::string> tmp_key(new_node->key);
    new_node->prev = _lru_tail;
    if(_lru_tail != nullptr)
        _lru_tail->next = std::unique_ptr<lru_node>(new_node);
    _lru_index.insert(std::make_pair(tmp_key, wrapped_node));
    _lru_tail = new_node;
    _capacity -= key.size() + value.size();
}

void SimpleLRU::ChangeNodeValue(lru_node& node, const std::string& value) {
    _capacity = _capacity + node.value.size() - value.size();
    if (node.next == nullptr) {
        node.value = value;
        return;
    }
    if(node.prev == nullptr) {
        std::unique_ptr<lru_node> tmp_ptr = std::move(_lru_head);
        _lru_head = std::move(tmp_ptr->next);
        _lru_head->prev = nullptr;
        node.prev = _lru_tail;
        node.next = nullptr;
        _lru_tail->next = std::move(tmp_ptr);
        _lru_tail = &node;
    }
    else {
        node.next->prev = node.prev;
        node.prev = _lru_tail;
        std::unique_ptr<lru_node> tmp_ptr = std::move(node.next->prev->next);
        node.next->prev->next = std::move(node.next);
        _lru_tail->next = std::move(tmp_ptr);
        node.next = nullptr;
        _lru_tail = &node;
    }
}

} // namespace Backend
} // namespace Afina
