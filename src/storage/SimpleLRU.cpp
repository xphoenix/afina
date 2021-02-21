#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

void SimpleLRU::update_keynode(SimpleLRU::lru_node & cur_node, const std::string &value)
{
    while (_nonblank_size - cur_node.value.size() + value.size() > _max_size) {
        SimpleLRU::Delete(_lru_tail->key);
    }
    _nonblank_size += - cur_node.value.size() + value.size();
    cur_node.value = value;
    go_to_head(cur_node);
    return;
}

void SimpleLRU::add_new_pair(const std::string &key, const std::string &value)
{
    while (_nonblank_size + key.size() + value.size() > _max_size) {
        SimpleLRU::Delete(_lru_tail->key);
    }
    SimpleLRU::lru_node * new_node = new SimpleLRU::lru_node{key, value, nullptr, nullptr};
    if (_lru_head != nullptr)
    {
        _lru_head->prev = new_node;
    }
    if (_lru_tail == nullptr)
    {
        _lru_tail = new_node;
    }
    _nonblank_size += key.size() + value.size();
    new_node->next = std::move(_lru_head);
    _lru_head = std::unique_ptr<SimpleLRU::lru_node>(new_node);
    _lru_index.emplace(std::make_pair(std::reference_wrapper<const std::string>(new_node->key), std::reference_wrapper<SimpleLRU::lru_node>(*new_node)));
    go_to_head(*new_node);
    return;
}

void SimpleLRU::del_node(SimpleLRU::lru_node & cur_node)
{
    if (cur_node.prev != nullptr){
       if(cur_node.next != nullptr){
           cur_node.next->prev = cur_node.prev;
           cur_node.prev->next = std::move(cur_node.next);
       } else {
           _lru_tail = cur_node.prev;
           cur_node.prev->next = std::move(cur_node.next);
       }
   } else {
       if (cur_node.next != nullptr){
           cur_node.next->prev = cur_node.prev;
           _lru_head = std::move(cur_node.next);
       } else {
           _lru_tail = cur_node.prev;
           _lru_head = std::move(cur_node.next);
       }
   }
   return;
}

void SimpleLRU::go_to_head(SimpleLRU::lru_node & cur_node)
{
    if (_lru_head.get() != & cur_node)
    {
        auto one_more_ptr = std::move(cur_node.prev->next);
        one_more_ptr->prev->next = std::move(one_more_ptr->next);
        one_more_ptr->prev = nullptr;
        if (one_more_ptr->next == nullptr) {
            _lru_tail = one_more_ptr->prev;
        } else {
            one_more_ptr->next->prev = one_more_ptr->prev;
        }
        one_more_ptr->next = std::move(_lru_head);
        one_more_ptr->next->prev = one_more_ptr.get();
        _lru_head = std::move(one_more_ptr);
    }
    return;
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto find_key = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (find_key != _lru_index.end()) {
        update_keynode(find_key->second, value);
    } else {
        add_new_pair(key, value);
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    if (_lru_index.find(std::reference_wrapper<const std::string>(key)) != _lru_index.end()) {
        return false;
    }
    add_new_pair(key, value);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto find_key = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (find_key == _lru_index.end()) {
        return false;
    }
    update_keynode(find_key->second, value);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    if (_lru_head == nullptr) {
        return false;
    }
    auto find_key = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (find_key == _lru_index.end()) {
        return false;
    }
    std::size_t del_size = find_key->second.get().key.size() + find_key->second.get().value.size();
    auto cur_del_node = find_key->second;
    _lru_index.erase(find_key);
    _nonblank_size -= del_size;
    SimpleLRU::del_node(cur_del_node);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto find_key = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (find_key == _lru_index.end()) {
        return false;
    }
    value = find_key->second.get().value;
    go_to_head(find_key->second);
    return true;
}

} // namespace Backend
} // namespace Afina
