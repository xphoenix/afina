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
        // TODO: delete nodes from tail until enough space
        // and refresh _cur_size
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

    // TODO: error
    if (it->second.get().prev && it->second.get().next){
        // // move founded node to head
        // create new pnode
        std::unique_ptr<lru_node> pnode;

        // link pnode to head & head to founded
        pnode = std::move(_lru_head);
        _lru_head = std::move(it->second.get().prev->next);

        // prev.next to next & next.prev to prev
        it->second.get().prev->next = std::move(it->second.get().next);
        it->second.get().prev->next->prev = it->second.get().prev; // 'delete' this branch for setting at last node

        // if size of new node is too big
        if (_cur_size - old_size + new_size > _max_size) {
            // TODO: delete nodes from tail until enough space
            // and refresh _cur_size
        }

        it->second.get().next = std::move(pnode);
        it->second.get().prev = nullptr;
    }

    _lru_head->key = key;
    _lru_head->value = value;

    // update _cur_size ?

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
    }  // first, not last
    else if (it->second.get().next && !it->second.get().prev) {
        _lru_head = std::move(it->second.get().next);
        _lru_head->prev = nullptr;
    }

    // refresh _cur_size
    _cur_size -= d_size;

    //update _lru_index
    _lru_index.erase(it);

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
