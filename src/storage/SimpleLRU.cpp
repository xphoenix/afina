#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

void SimpleLRU::move_to_tail(lru_node &node) {
    std::unique_ptr<lru_node> tmp = std::move(node.prev->next);
    remove_node(tmp.get(), false);
    tmp->prev = _lru_tail->prev;
    tmp->next = std::move(_lru_tail->prev->next);
    _lru_tail->prev = tmp.get();
    tmp->prev->next = std::move(tmp);
}

void SimpleLRU::append_node(const std::string &key, const std::string &value) {
    std::unique_ptr<lru_node> node = std::make_unique<lru_node>();
    node->key = key;
    node->value = value;
    _cur_size += key.size() + value.size();

    node->prev = _lru_tail->prev;
    node->next = std::move(_lru_tail->prev->next);

    _lru_tail->prev->next = std::move(node);
    lru_node *last_elem = _lru_tail->prev->next.get();
    _lru_tail->prev = last_elem;
    _lru_index.insert(std::pair<const std::string &, lru_node &>(_lru_tail->prev->key, *last_elem));
}

void SimpleLRU::remove_node(lru_node *node, bool erase = true) {
    if (erase) {
        _lru_index.erase(node->key);
        _cur_size -= node->key.size() + node->value.size();
    }
    lru_node *left = node->prev;
    left->next = std::move(node->next);
    left->next->prev = left;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    size_t size = key.size() + value.size();
    if (size > _max_size) {
        return false;
    }
    auto pair = _lru_index.find(key);
    if (pair == _lru_index.end()) {
        while (_cur_size + size > _max_size) {
            remove_node(_lru_head->next.get());
        }
        append_node(key, value);
    } else {
        lru_node &tmp = pair->second.get();
        while (_cur_size - tmp.value.size() + value.size() > _max_size) {
            remove_node(_lru_head->next.get());
        }
        tmp.value = value;
        move_to_tail(tmp);
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    auto pair = _lru_index.find(key);
    if (pair == _lru_index.end()) {
        return Put(key, value);
    } else {
        return false;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto pair = _lru_index.find(key);
    if (pair == _lru_index.end()) {
        return false;
    }
    lru_node &tmp = pair->second.get();
    if (_cur_size - tmp.value.size() + value.size() > _max_size) {
        return false;
    }
    _cur_size += value.size() - tmp.value.size();
    tmp.value = value;
    move_to_tail(tmp);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto pair = _lru_index.find(key);
    if (pair == _lru_index.end()) {
        return false;
    }
    remove_node(&pair->second.get());
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto pair = _lru_index.find(key);
    if (pair == _lru_index.end()) {
        return false;
    }
    lru_node &tmp = pair->second.get();
    value = tmp.value;
    move_to_tail(tmp);
    return true;
}
} // namespace Backend
} // namespace Afina