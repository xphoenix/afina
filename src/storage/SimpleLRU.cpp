#include "SimpleLRU.h"

namespace Afina {
    namespace Backend {

        void SimpleLRU::delete_node(lru_node *node) {

            _lru_index.erase(node->key);
            _curr_size -= (node->key.size() + node->value.size());

            // если 1 элемент в кэше
            if (!node->prev && !node->next) {

                _lru_head.reset();
                _lru_tail = 0;

            // если head
            } else if (!node->prev) {

                node->next->prev = 0;
                _lru_head = std::move(node->next);
                node->next.reset();
            }

            // если tail
            else if (!node->next) {

                node->prev->next.reset();
                _lru_tail = node->prev;
                node->prev = 0;
            }

            // если в середине
            else {
                node->next->prev = node->prev;
                node->prev->next = std::move(node->next);
            }
        }

        void SimpleLRU::append_node(const std::string &key, const std::string &value) {

            std::unique_ptr<lru_node> new_node(new lru_node(key, value));

            if (!_lru_tail) {

                _lru_tail = new_node.get();
                _lru_head = std::move(new_node);

            } else {

                // tail указывает на новую ноду
                _lru_tail->next = std::move(new_node);

                // новая нода указывает на "старый" tail
                _lru_tail->next->prev = _lru_tail;

                // новый tail - новая нода
                _lru_tail = _lru_tail->next.get();
            }

            _lru_index.emplace(_lru_tail->key, *_lru_tail);
            _curr_size += _lru_tail->key.size() + _lru_head->value.size();
        }

        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Put(const std::string &key, const std::string &value) {

            std::size_t new_node_size = key.size() + value.size();

            if (new_node_size > _max_size) {
                return false;
            }

            auto it = _lru_index.find(key);

            // если ключа нет в кэше
            if (it == _lru_index.end()) {
                // удаляем голову, пока не будет свободного места
                while (_curr_size + new_node_size > _max_size) {

                    delete_node(_lru_head.get());
                }

                append_node(key, value);

            } else {

                it->second.get().value = value;
            }

            return true;
        }

        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {

            auto it = _lru_index.find(key);

            if (it == _lru_index.end()) {
                append_node(key, value);
                return true;
            }

            return false;
        }

        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Set(const std::string &key, const std::string &value) {

            auto it = _lru_index.find(key);

            if (it == _lru_index.end()) {
                return false;
            }

            // удаляем голову, пока не будет свободного места
            while (_curr_size - it->second.get().value.size() + value.size() > _max_size) {
                delete_node(_lru_head.get());
            }

            _curr_size += value.size() - it->second.get().value.size();

            delete_node(&it->second.get());
            append_node(key, value);


            return true;
        }

        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Delete(const std::string &key) {

            auto it = _lru_index.find(key);

            if (it == _lru_index.end()) {
                return false;
            }

            delete_node(&it->second.get());

            return true;
        }

        // See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Get(const std::string &key, std::string &value) {

            auto it = _lru_index.find(key);

            if (it == _lru_index.end()) {
                return false;
            }

            value = it->second.get().value;

            delete_node(&it->second.get());
            append_node(key, value);

            return true;
        }

    } // namespace Backend
} // namespace Afina


