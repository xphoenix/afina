#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {

/**
 * # Map based implementation
 * That is NOT thread safe implementaiton!!
 */
class SimpleLRU : public Afina::Storage {
public:
    
    SimpleLRU(size_t max_size = 1024) : _max_size(max_size), _current_size(0), _lru_head(nullptr), _lru_tail(nullptr) {}

     ~SimpleLRU() {
        if (_lru_head) {
            _lru_index.clear();
            while (_lru_head->next != nullptr) {
                std::unique_ptr<lru_node> node_to_delete;
                node_to_delete.swap(_lru_head);
                _lru_head.swap(node_to_delete->next);
            }
            _lru_head.reset();
        }
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
    u using lru_node = struct lru_node {
        std::string key;
        std::string value;
        lru_node *prev;
        std::unique_ptr<lru_node> next;
        lru_node(const std::string &k, const std::string &v) : key(k), value(v), prev(nullptr), next(nullptr) {}
    };
    using mapT = std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>,
                          std::less<const std::string>>;
    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size, _current_size;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    lru_node *_lru_tail;
    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    mapT _lru_index;

    bool put_node(const std::string &key, const std::string &value);
    bool update_node(const mapT::iterator &it, const std::string &value);
    bool remove_node(lru_node &delete_node);
    bool move_node_tail(lru_node &node);
};
} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
