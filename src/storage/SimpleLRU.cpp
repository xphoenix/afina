#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

void SimpleLRU::pop_front() {
    lru_node &node = *head;
    std::unique_ptr<lru_node> temp;
    if (node.next) {
        node.next.get()->prev = nullptr;
    }
    temp.swap(head);
    head = std::move(node.next);
    _lru_index.erase(node.key);
    _current_size -= node.key.size() + node.value.size();
}

void SimpleLRU::push_back(const std::string &key, const std::string &value) {
    std::unique_ptr<lru_node> temp(new lru_node{key, value});
    if (head.get() != nullptr) {
        temp->prev = tail;
        tail->next.swap(temp);
        tail = tail->next.get();
    } else {
        head.swap(temp);
        tail = head.get();
    }
}

void SimpleLRU::move_back(lru_node &node) {
    if (tail == &node) {
        return;
    }
    if (head.get() == &node) {
        head.swap(node.next);
        head->prev = nullptr;
        tail->next.swap(node.next);
        node.prev = tail;
        tail = &node;
    } else {
        node.next->prev = node.prev;
        node.prev->next.swap(node.next);

        // Now node.next points to the NODE
        tail->next.swap(node.next);
        node.prev = tail;
        tail = &node;
    }
}

void SimpleLRU::add(const std::string &key, const std::string &value) {
    auto requested_size = key.size() + value.size();
    while (requested_size + _current_size > _max_size) {
        pop_front();
    }
    // Update list
    push_back(key, value);

    // Update index
    _lru_index.emplace(std::make_pair(std::ref(tail->key), std::ref(*tail)));
    _current_size += requested_size;
}

void SimpleLRU::remove(lru_node &node) {
    std::unique_ptr<lru_node> temp;
    if (node.prev == nullptr) { // remove head
        temp.swap(head);
        head = std::move(node.next);
        return;
    }
    if (node.next == nullptr) { // remove tail
        temp.swap(node.prev->next);
        node.prev->next = nullptr;
        tail = node.prev;
        return;
    }
    node.next->prev = node.prev;
    temp.swap(node.prev->next);
    node.prev->next = std::move(node.next);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    auto requested_size = key.size() + value.size();
    if (requested_size > _max_size) {
        return false;
    }
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        add(key, value);
    } else {
        lru_node &node = it->second;
        move_back(node);
        while (value.size() - it->second.get().value.size() + _current_size > _max_size) {
            pop_front();
        }
        _current_size += value.size() - it->second.get().value.size();
        node.value = value;
        
        return true;
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    if (_lru_index.find(key) != _lru_index.end()) {
        return false;
    } else {
        add(key, value);
        return true;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    } else {
        lru_node &node = it->second;
        move_back(node);
        while (value.size() - it->second.get().value.size() + _current_size > _max_size) {
            pop_front();
        }
        _current_size += value.size() - it->second.get().value.size();
        node.value = value;
        return true;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }

    // Update list
    lru_node &node = it->second;
    _current_size += node.key.size() + node.value.size();
    _lru_index.erase(it);

    remove(node);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) {
        return false;
    }
    value = it->second.get().value;
    lru_node &node = it->second;
    move_back(node);
    return true;
}

} // namespace Backend
} // namespace Afina
