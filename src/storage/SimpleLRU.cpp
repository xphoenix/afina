#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// Delete the nodes of list until the _lru_cash_list is able to store object of size "size".
// Return false if _max_size <= size;
bool SimpleLRU::cache_list_trim(size_t size) {
    if (size > _max_size) {
        return false;
    }

    while (_max_size - _current_size < size) {
        auto deleted_pair = _lru_cash_list.back();
        _lru_index.erase(deleted_pair->key);
        _current_size -= deleted_pair->key.size() + deleted_pair->value.size();
        _lru_cash_list.pop_back();
    }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        size_t pair_size = key.size() + value.size();

        if (!cache_list_trim(pair_size)) {
            return false;
        }

        _lru_cash_list.push_front(key, value);
        _lru_index.emplace(std::make_pair(std::ref(_lru_cash_list.front()->key), std::ref(*_lru_cash_list.front())));
        _current_size += key.size() + value.size();
        return true;
    } else {
        size_t old_value_size = it->second.get().value.size();
        size_t new_value_size = value.size();
        if (old_value_size >= new_value_size) {
            _current_size -= (it->second.get().value.size() - value.size());
            it->second.get().value = value;
            return true;
        } else {
            if (!cache_list_trim(new_value_size - old_value_size)) {
                return false;
            }
            it->second.get().value = value;
            return true;
        }
    }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (_lru_index.count(key) == 0) {
        return Put(key, value);
    } else {
        return false;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        return false;
    } else {
        return Put(key, value);
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        return false;
    } else {
        _lru_cash_list.erase(&it->second.get());
        _lru_index.erase(it);
        return true;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        return false;
    } else {
        value = it->second.get().value;
        return true;
    }
}

void Lru_cash_list::push_front(const std::string &key, const std::string &value) {
    if (_lru_head == nullptr) {
        _lru_head = std::unique_ptr<lru_node>(new lru_node(key, value));
        _lru_head->prev = _lru_head.get();
    } else {
        auto new_head = std::unique_ptr<lru_node>(new lru_node(key, value));
        new_head->prev = _lru_head->prev;
        new_head->next = std::move(_lru_head);
        new_head->next->prev = new_head.get();
        _lru_head = std::move(new_head);
    }
}

Lru_cash_list::lru_node *Lru_cash_list::back() const { return _lru_head->prev; }

Lru_cash_list::lru_node *Lru_cash_list::front() const { return _lru_head.get(); }

void Lru_cash_list::pop_back() {
    auto &new_last_node = _lru_head->prev->prev;
    _lru_head->prev = new_last_node;
    new_last_node->next = nullptr;
}

void Lru_cash_list::pop_front() {
    auto &new_head = _lru_head->next;
    new_head->prev = _lru_head->prev;
    _lru_head = std::move(new_head);
}

void Lru_cash_list::erase(lru_node *node) {
    if (node == _lru_head.get()) {
        pop_front();
    } else if (node == _lru_head->prev) {
        pop_back();
    } else {
        node->next->prev = node->prev;
        node->prev->next = std::move(node->next);
        node = nullptr;
    }
}

void Lru_cash_list::reset() {
    if (_lru_head != nullptr) {
        auto tmp = _lru_head->prev;
        while (tmp != _lru_head.get()) {
            tmp = tmp->prev;
            tmp->next = nullptr;
        }
        _lru_head = nullptr;
    }
}
} // namespace Backend
} // namespace Afina
