#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {

    if (key.size() + value.size() > max_size) {
        return false;
    }

    try {
        auto key_pointer = lru_index.find(key);
        if (key_pointer == lru_index.end()) {
            free_space(current_size - max_size + key.size() + value.size());
            make_new_node(key, value);
        } else {
            move_in_head(key_pointer->second.get());
            free_space(current_size - max_size + value.size() - lru_head->value.size());
            current_size += value.size() - lru_head->value.size();
            lru_head->value = value;
        }
    } catch (...) {
        return false;
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {

    if (key.size() + value.size() > max_size) {
        return false;
    }

    try {
        auto key_pointer = lru_index.find(key);
        if (key_pointer == lru_index.end()) {
            free_space(current_size - max_size + key.size() + value.size());
            make_new_node(key, value);
        } else {
            return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {

    if (key.size() + value.size() > max_size) {
        return false;
    }

    try {
        auto key_pointer = lru_index.find(key);
        if (key_pointer != lru_index.end()) {
            move_in_head(key_pointer->second.get());
            free_space(current_size - max_size + value.size() - lru_head->value.size());
            current_size += value.size() - lru_head->value.size();
            lru_head->value = value;
        } else {
            return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {

    try {
        auto key_pointer = lru_index.find(key);
        if (key_pointer != lru_index.end()) {

            lru_node* node_to_delete = &(key_pointer->second.get());
            lru_index.erase(key_pointer->first);
            current_size -= key.size() + node_to_delete->value.size();

            if (lru_head.get() == node_to_delete) {
                if (lru_head.get() == last_node) {
                    last_node = nullptr;
                }
                lru_head = std::move(lru_head->next);
            } else {

                if (static_cast<bool>(node_to_delete->next)) {
                    node_to_delete->next->prev = node_to_delete->prev;
                } else {
                    last_node = node_to_delete->prev;
                }

                node_to_delete->prev->next = std::move(node_to_delete->next);
            }

        } else { return false; }
    } catch (...) { return false; }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    try {
        auto key_pointer = lru_index.find(key);
        if (key_pointer != lru_index.end()) {
            value = key_pointer->second.get().value;
            move_in_head(key_pointer->second.get());
        } else {
            return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

void SimpleLRU::free_space(int64_t bytes_needed) {
    while (bytes_needed > 0) {
        bytes_needed -= last_node->key.size() + last_node->value.size();
        Delete(last_node->key);
    }
}

void SimpleLRU::make_new_node(const std::string &key, const std::string &value) {
    if (static_cast<bool>(lru_head)) {
        lru_head->prev = new lru_node{key, value, nullptr, nullptr};
        lru_node *tmp = lru_head.release();
        tmp->prev->next.reset(tmp);
        lru_head.reset(tmp->prev);
    } else {
        lru_head = std::unique_ptr<lru_node>(new lru_node{key, value, nullptr, nullptr});
        last_node = lru_head.get();
    }

    current_size += key.size() + value.size();
    lru_index.insert(std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>>(std::ref(lru_head->key), std::ref(*lru_head)));
}

void SimpleLRU::move_in_head(lru_node &node) {

    if (lru_head.get() != &node) {

        if (static_cast<bool>(node.next)) {
            node.next->prev = node.prev;
        } else {
            last_node = node.prev;
        }

        node.prev->next.release();
        node.prev->next = std::move(node.next);
        node.prev = nullptr;
        lru_node *tmp = lru_head.release();
        node.next.reset(tmp);
        tmp->prev = &node;
        lru_head.reset(tmp->prev);
    }
}

} // namespace Backend
} // namespace Afina
