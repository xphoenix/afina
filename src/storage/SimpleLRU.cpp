#include "SimpleLRU.h"

namespace Afina {
    namespace Backend {

// See MapBasedGlobalLockImpl.h

        bool SimpleLRU::Put(const std::string &key, const std::string &value) {
            std::size_t pair_size = key.size() + value.size();
            if (pair_size > _max_size) {
                return false;
            }
            auto it = _lru_index.find(key);
            if (it != _lru_index.end()) {
                lru_node &node = it->second.get();
                if (node.prev && node.next) {
                    node.next->prev = node.prev;
                    _lru_tail->next = std::move(node.prev->next);
                    node.prev->next = std::move(node.next);
                    _lru_tail->next->prev = _lru_tail;
                    _lru_tail = _lru_tail->next.get();
                } else if (!node.prev && _lru_index.size() > 1) {
                    _lru_head->next->prev = nullptr;
                    _lru_head->prev = _lru_tail;
                    _lru_tail->next = std::move(_lru_head);
                    _lru_head = std::move(node.next);
                    _lru_tail = _lru_tail->next.get();
                }
                storage_size += value.size() - node.value.size();
                while (storage_size > _max_size) {
                    Delete(_lru_head->key);
                }
                node.value.erase();
                node.value = value;
            } else {
                std::unique_ptr<lru_node> new_node(new lru_node);
                new_node->key = key;
                new_node->value = value;
                new_node->next = nullptr;

                storage_size += pair_size;
                while (storage_size > _max_size) {
                    Delete(std::ref(_lru_head->key));
                }

                if (_lru_index.empty()) {
                    new_node->prev = nullptr;
                    _lru_index.insert(std::make_pair(std::ref(new_node->key), std::ref(*new_node)));
                    _lru_head = std::move(new_node);
                    _lru_tail = _lru_head.get();
                } else {
                    new_node->prev = _lru_tail;
                    _lru_index.insert(std::make_pair(std::ref(new_node->key), std::ref(*new_node)));
                    _lru_tail->next = std::move(new_node);
                    _lru_tail = _lru_tail->next.get();
                }
            }
            return true;
        }

// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
            auto it = _lru_index.find(key);
            if (it == _lru_index.end()) {
                return Put(key, value);
            }
            return false;
        }

// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Set(const std::string &key, const std::string &value) {
            auto it = _lru_index.find(key);
            if (it != _lru_index.end()) {
                return Put(key, value);
            }
            return false;
        }

// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Delete(const std::string &key) {
            auto it = _lru_index.find(key);
            if (it != _lru_index.end()) {
                lru_node &node = it->second.get();
                storage_size -= node.key.size();
                storage_size -= node.value.size();

                if (node.prev) {
                    if (node.next) {
                        node.next->prev = node.prev;
                    }
                    node.prev->next = std::move(node.next);
                } else {
                    _lru_head = std::move(_lru_head->next);
                    _lru_head->prev = nullptr;
                }
                _lru_index.erase(it);
                return true;
            }
            return false;
        }

// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Get(const std::string &key, std::string &value) {
            auto it = _lru_index.find(key);
            if (it != _lru_index.end()) {
                value = it->second.get().value;
                return true;
            }
            return false;
        }

    } // namespace Backend
} // namespace Afina