#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
    if (this->_lru_index.find(key) == this->_lru_index.end()) {
        return this->_Put(key, value);
    }
    return this->_UpdateValue(this->_lru_index.find(key)->second.get(), value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    if (this->_lru_index.find(key) == this->_lru_index.end()) {
        return this->_Put(key, value);
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    if (this->_lru_index.find(key) == this->_lru_index.end()) {
        return false;
    }
    return this->_UpdateValue(this->_lru_index.find(key)->second.get(), value);
}
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
    if (this->_lru_index.find(key) == this->_lru_index.end()) {
        return false;
    }
    this->_DeleteNode(this->_lru_index.find(key)->second.get());
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value)
{
    if (this->_lru_index.find(key) == this->_lru_index.end()) {
        return false;
    }
    lru_node &node = this->_lru_index.find(key)->second.get();
    value = node.value;
    this->_MoveNode(node);
    return true;
}

bool SimpleLRU::_Put(const std::string &key, const std::string &value)
{
    std::size_t size = key.size() + value.size();
    if (size > this->_max_size) {
        return false;
    }
    while (this->_size + size > this->_max_size) {
        this->_DeleteNode(std::ref(*(this->_lru_head)));
    }
    lru_node *new_node = new lru_node { key, value, this->_lru_tail, nullptr };
    if (this->_lru_head) {
        this->_lru_tail->next = std::unique_ptr<lru_node>(new_node);
    } else {
        this->_lru_head = std::unique_ptr<lru_node>(new_node);
    }
    this->_size += size;
    this->_lru_tail = new_node;
    this->_lru_index.emplace(std::cref(new_node->key), std::ref(*new_node));
    return true;
}

bool SimpleLRU::_UpdateValue(lru_node &node, const std::string &value)
{
    std::size_t old_value_size = node.value.size();
    std::size_t value_size = value.size();
    std::size_t old_size = old_value_size + node.key.size();
    if (value_size + node.key.size() > this->_max_size) {
        return false;
    }
    //node.value = value;
    this->_MoveNode(node);
    while (this->_size + value_size > this->_max_size + old_value_size) {
        this->_DeleteNode(std::ref(*(this->_lru_head)));
    }
    this->_size += value_size - old_value_size;
    node.value = value;
    return true;
}

void SimpleLRU::_DeleteNode(lru_node &node) {
    std::size_t size = node.key.size() + node.value.size();
    this->_size -= size;
    this->_lru_index.erase(node.key);
    if (this->_lru_head.get() == this->_lru_tail) {
        this->_lru_head = nullptr;
        this->_lru_tail = nullptr;
    } else if (!node.prev) {
        this->_lru_head = std::move(this->_lru_head->next);
        this->_lru_head->prev = nullptr;
    } else if (!node.next) {
        this->_lru_tail = this->_lru_tail->prev;
        this->_lru_tail->next = nullptr;
    } else {
        lru_node *prev_node = node.prev;
        prev_node->next = std::move(node.next);
        prev_node->next->prev = prev_node;
    }
}

bool SimpleLRU::_MoveNode(lru_node &node)
{
    if (node.next) {
        // Change next
        lru_node *ptr = node.next->prev;
        ptr->next->prev = ptr->prev;
        if (!ptr->prev) {
            //Change head and head
            this->_lru_tail->next = std::move(this->_lru_head); // Why std::move?
            this->_lru_head = std::move(ptr->next);
        } else {
            //Change tail and prev
            this->_lru_tail->next = std::move(ptr->prev->next);
            ptr->prev->next = std::move(ptr->next);
        }
        ptr->prev = this->_lru_tail;
        this->_lru_tail = nullptr;
        ptr->next = nullptr;
    } else {
        /**/
    }
}

} // namespace Backend
} // namespace Afina
