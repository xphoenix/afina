#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <map>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <iostream>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {

/**
 * # Map based implementation
 * That is NOT thread safe implementaiton!!
 */
class SimpleLRU : public Afina::Storage {
public:
    SimpleLRU(size_t max_size = 1024) : _max_size(max_size), _capacity(max_size) {}

    ~SimpleLRU() {
        _lru_index.clear();
        if(_lru_head != nullptr) {
            _lru_tail = _lru_tail->prev;
            while(_lru_tail != nullptr) {
                _lru_tail->next.reset();
                _lru_tail = _lru_tail->prev;
            }
            _lru_head.reset();
        }
        ;
    }

    // Implements Afina::Storage interface
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    // Seems to be done
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
    };

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;
    //capacity that remains
    std::size_t _capacity;
    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    // List owns all nodes
    std::unique_ptr<lru_node> _lru_head = nullptr;
    lru_node* _lru_tail = nullptr;
    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<std::string>> _lru_index;

    // Some helpful functions
    void DeleteWorstNode();
    void InsertNodeToEmptyList(const std::string &key, const std::string &value);
    void InsertNode(const std::string &key, const std::string &value);
    void ChangeNodeValue(lru_node& node, const std::string& value);

};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
