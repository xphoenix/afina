#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::_overflow(size_t new_size) const
{
    return new_size > _max_size;
}

// See MapBasedGlobalLockImpl.h
void SimpleLRU::_insert_node(std::unique_ptr<lru_node> &node)
{
    if (_lru_head->next == nullptr) {
        _lru_head->next = std::move(node);
        _lru_head->next->prev = _lru_head.get();
    } else {
        _lru_head->next->prev = node.get();
        node->next = std::move(_lru_head->next);
        node->prev = _lru_head.get();
        _lru_head->next = std::move(node);
    }
}

// See MapBasedGlobalLockImpl.h
void SimpleLRU::_get_up(lru_node *cur)
{
    // if cur already in head
    if (cur == _lru_head->next.get()) {
        return;
    }

    _lru_head->next->prev = cur;
    if (cur->next != nullptr) {
        cur->next->prev = cur->prev;
    }

    // update next of head
    lru_node *second = _lru_head->next.release();
    _lru_head->next.reset(cur);

    // update next of prev_of_current
    cur->prev->next.release();
    lru_node *center_next = cur->next.release();
    cur->prev->next.reset(center_next);

    // insert current after head
    cur->prev = _lru_head.get();
    cur->next.release();
    cur->next.reset(second);
}


bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
    auto it_find = _lru_index.find(std::cref(key));
    if (it_find == _lru_index.end()) {
        if (_overflow(_cur_size + key.size() + value.size())) {
            return false;
        }

        auto node = std::unique_ptr<lru_node>( new lru_node(key, value) );

        auto pair = _lru_index.emplace(std::make_pair(std::cref(node->key),
                                                      std::ref(*node)));
        auto it_node = pair.first;
        bool res = pair.second;
        if (!res) {
            return false;
        }

        _cur_size += key.size() + value.size();

        // insert node in the top of the list
        _insert_node(node);

    } else {
        auto node_ref = it_find->second;
        size_t node_value_size = node_ref.get().value.size();

        if (_overflow(_cur_size - node_value_size + value.size())) {
            return false;
        }

        _cur_size = _cur_size - node_ref.get().value.size() + value.size();
        node_ref.get().value = value;

        lru_node *cur = _lru_head->next.get();
        while (cur != nullptr) {
            if (cur->key == key) {
                _get_up(cur);
                break;
            }
            cur = cur->next.get();
        }
    }
    return true;
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    auto it_find = _lru_index.find(std::cref(key));
    if (it_find == _lru_index.end()) {
        if (_overflow(_cur_size + key.size() + value.size())) {
            return false;
        }

        auto node = std::unique_ptr<lru_node>( new lru_node(key, value) );

        auto pair = _lru_index.emplace(std::make_pair(std::cref(node->key), std::ref(*node)));
        auto it_node = pair.first;
        bool res = pair.second;
        if (!res) {
            return false;
        }

        _cur_size += key.size() + value.size();

        // insert node in the top of the list
        _insert_node(node);

    } else {
        return false; // or true?
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    auto it_find = _lru_index.find(std::cref(key));
    if (it_find != _lru_index.end()) {
        auto node_ref = it_find->second;
        size_t node_value_size = node_ref.get().value.size();

        if (_overflow(_cur_size - node_value_size + value.size())) {
            return false;
        }

        _cur_size = _cur_size - node_ref.get().value.size() + value.size();
        node_ref.get().value = value;

        lru_node *cur = _lru_head->next.get();
        while (cur != nullptr) {
            if (cur->key == key) {
                _get_up(cur);
                break;
            }
            cur = cur->next.get();
        }
    } else {
        return false;
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
    auto it_find = _lru_index.find(std::cref(key));
    if (it_find == _lru_index.end()) {
        return false; // or true?
    }

    lru_node *cur = _lru_head->next.get();
    while (cur != nullptr) {
        if (cur->key == key) {
            _cur_size -= cur->value.size();
            if (cur->next != nullptr) {
                cur->next->prev = cur->prev;
                cur->prev->next = std::move(cur->next);
            } else {
                cur->prev->next = nullptr;
            }

            // cur->prev->next.reset(cur->next.get());
            break;
        }
        cur = cur->next.get();
    }

    _lru_index.erase(it_find);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value)
{
    auto it_find = _lru_index.find(std::cref(key));
    if (it_find == _lru_index.end()) {
        return false;
    } else {
        lru_node *cur = _lru_head->next.get();
        while (cur != nullptr) {
            if (cur->key == key) {
                _get_up(cur);
                value = cur->value;
                break;
            }
            cur = cur->next.get();
        }
    }
    return true;
}

} // namespace Backend
} // namespace Afina
