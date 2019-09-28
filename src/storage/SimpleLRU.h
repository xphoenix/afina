#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <afina/Storage.h>

namespace Afina {
    namespace Backend {
        /**
         * # Map based implementation
         * That is NOT thread safe implementaiton!!
         */
        class SimpleLRU : public Afina::Storage {
        public:
            SimpleLRU(size_t capacity = 1024)
                : _capacity(capacity), _size(0), _lru_tail(nullptr) {}

            ~SimpleLRU() {
                _lru_index.clear();
                if (_lru_tail != nullptr)
                    for(_lru_tail = _lru_tail->prev; _lru_tail != nullptr; _lru_tail = _lru_tail->prev)
                        _lru_tail->next.reset();
            }

            // Implements Afina::Storage interface
            bool Put(const std::string &key, const std::string &value) override;

            // Implements Afina::Storage interface
            bool PutIfAbsent(const std::string &key, const std::string &value) override;

            // Implements Afina::Storage interface
            bool Set(const std::string &key, const std::string &value) override;

            // Implements Afina::Storage interface
            bool Delete(const std::string &key) override;

            // Implements Afina::Storage interface
            bool Get(const std::string &key, std::string &value) override;

        private:
            // LRU cache node
            using lru_node = struct lru_node {
                const std::string key;
                std::string value;
                lru_node* prev;
                std::unique_ptr<lru_node> next;

                lru_node(const std::string &key_, const std::string &value_)
                    : key(key_), value(value_), prev(nullptr) {}
            };

            // Maximum number of bytes could be stored in this cache.
            // i.e all (keys+values) must be less the _capacity
            std::size_t _capacity;
            std::size_t _size;
            // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
            // element that wasn't used for longest time.
            //
            // List owns all nodes
            std::unique_ptr<lru_node> _lru_head;
            lru_node* _lru_tail;

            // Index of nodes from list above, allows fast random access to elements by lru_node#key
            std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<std::string>> _lru_index;

            bool push(std::unique_ptr<lru_node> new_head);
            std::unique_ptr<lru_node> pop(lru_node* node);
        };

    } // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
