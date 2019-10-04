#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()){
        return Set(key, value);
    }
    else {
        return PutIfAbsent(key, value);
    }
}

// delete nodes from tail until enough space
// and refresh _cur_size
void SimpleLRU::free_memmory_for_node(const std::string &key, const std::string &value){
  lru_node *p = _lru_head.get();
  if (p){
      while (p->next){
        p = p->next.get();
      }
      while(p->prev && (_cur_size + key.size() + value.size() > _max_size)){
          std::string k = p->key;
          size_t csz = p->key.size() + p->value.size();
          _lru_index.erase(k);
          p = p->prev;
          p->next.reset();
          _cur_size -= csz;
      }
  }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()){
        return false;
    }
    if (key.size() + value.size() > _max_size) {
        return false;
    }

    // if size of new node is too big
    if (_cur_size + key.size() + value.size() > _max_size) {
        free_memmory_for_node(key, value);
    }

    // create new node
    std::unique_ptr<lru_node> pnode(new lru_node);
    pnode->key = key;
    pnode->value = value;
    pnode->next = nullptr;
    pnode->prev = nullptr;

    // insert it in head
    if (_lru_head == nullptr) {
        _lru_head = std::move(pnode);
    }
    else {
        pnode->next = std::move(_lru_head);
        (pnode->next)->prev = pnode.get();
        _lru_head = std::move(pnode);
      }

    _lru_index.insert(std::make_pair(std::ref(_lru_head->key), std::ref(*_lru_head)));
    _cur_size += key.size() + value.size();
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        // ? and that is all ? anything changes ?
        return false;
    }
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    size_t new_size = key.size() + value.size();
    size_t old_size = it->second.get().key.size() + it->second.get().value.size();

    if (it->second.get().prev){
        // // move founded node to head
        // create new pnode
        std::unique_ptr<lru_node> pnode;

        // link pnode to head & head to founded
        pnode = std::move(_lru_head);
        _lru_head = std::move(it->second.get().prev->next);

        // prev.next to next & next.prev to
        if (it->second.get().next){
            it->second.get().prev->next = std::move(it->second.get().next);
            it->second.get().prev->next->prev = it->second.get().prev; // 'delete' this branch for setting at last node
        } else {
           it->second.get().prev->next = std::move(it->second.get().next);
        }

        // if size of new node is too big
        if (_cur_size - old_size + new_size > _max_size) {
            free_memmory_for_node(key, value);
        }

        it->second.get().next = std::move(pnode);
        it->second.get().prev = nullptr;
    }

    _lru_head->key = key;
    _lru_head->value = value;

    // update _cur_size
    _cur_size += key.size() + value.size();
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    size_t d_size = it->second.get().key.size() + it->second.get().value.size();

    // last, not first
    if (!it->second.get().next && it->second.get().prev) {
        it->second.get().prev->next = std::move(it->second.get().next);
        _lru_index.erase(it);
    }  // first, not last
    else if (it->second.get().next && !it->second.get().prev) {
        std::unique_ptr<lru_node> pnode;
        pnode = std::move(_lru_head);

        _lru_head = std::move(it->second.get().next);
        _lru_head->prev = nullptr;
        _lru_index.erase(it);
    }

    // refresh _cur_size
    _cur_size -= d_size;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    value = it->second.get().value;
    return true;
}

} // namespace Backend
} // namespace Afina
