#include "SimpleLRU.h"

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

namespace Afina {
    namespace Backend {
        bool SimpleLRU::push(std::unique_ptr<lru_node> new_head) {
            std::size_t size = new_head->value.size() + new_head->key.size();
            if (size > _capacity) return false;
            for (; size > _capacity - _size; pop(_lru_tail));
            if (_lru_head == nullptr) {
                _lru_head = std::move(new_head);
                _lru_tail = _lru_head.get();
            } else {
                _lru_head->prev = new_head.get();
                new_head->next = std::move(_lru_head);
                _lru_head = std::move(new_head);
            }
            _size += size;
            return true;
        }

        std::unique_ptr<SimpleLRU::lru_node> SimpleLRU::pop(lru_node* node) {
            _size -= node->key.size() + node->value.size();
            if (node == _lru_head.get()) {
                auto res = _lru_head.release();
                if (_lru_tail == _lru_head.get())
                    _lru_tail = nullptr;
                _lru_head = std::move(res->next);
                if (_lru_head != nullptr)
                    _lru_head->prev = nullptr;
                return std::unique_ptr<lru_node>(res);
            }
            if (node->next != nullptr)
                node->next->prev = node->prev;
            else
                _lru_tail = _lru_tail->prev;
            auto res = node->prev->next.release();
            node->prev->next = std::move(node->next);
            res->prev = nullptr;
            return std::unique_ptr<lru_node>(res);
        }


        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Put(const std::string &key, const std::string &value) {
            auto it = _lru_index.find(key);
            if (it == _lru_index.end()) {
                auto new_head = make_unique<lru_node>(key, value);
                if (push(std::move(new_head))) {
                    _lru_index.insert({std::ref(key), std::ref(*new_head.get())});
                    return true;
                }
            } else {
                if (key.size() + value.size() > _capacity)
                    return false;
                auto new_head = pop(&it->second.get());
                it->second.get().value = value;
                return push(std::move(new_head));
            }
            return false;
        }

        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
            auto it = _lru_index.find(key);
            if (it == _lru_index.end()) {
                if (key.size() + value.size() > _capacity)
                    return false;
                auto new_head = make_unique<lru_node>(key, value);
                if (push(std::move(new_head))) {
                    _lru_index.insert({std::ref(key), std::ref(*new_head.get())});
                    return true;
                }
            }
            return false;
        }

        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Set(const std::string &key, const std::string &value) {
            auto it = _lru_index.find(key);
            if (it != _lru_index.end()) {
                if (key.size() + value.size() > _capacity)
                    return false;
                auto new_head = pop(&it->second.get());
                it->second.get().value = value;
                return push(std::move(new_head));
            }
            return false;
        }

        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Delete(const std::string &key) {
            auto it = _lru_index.find(key);
            if (it != _lru_index.end()) {
                pop(&it->second.get());
                _lru_index.erase(it);
                return true;
            }
            return false;
        }

        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Get(const std::string &key, std::string &value) {
            auto it = _lru_index.find(key);
            if (it != _lru_index.end()) {
                auto new_head = pop(&it->second.get());
                value = new_head->value;
                return push(std::move(new_head));
            }
            return false;
        }

    } // namespace Backend
} // namespace Afina
