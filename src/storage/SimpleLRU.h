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
     SimpleLRU(size_t max_size = 1024) : _max_size(max_size)
     {
         _lru_head = std::unique_ptr<lru_node>( new lru_node("", "") );
         _lru_tail = _lru_head.get();
     }

     ~SimpleLRU() {
         _lru_index.clear();
         while (_lru_tail != _lru_head.get()) {
             _lru_tail = _lru_tail->prev;
             _lru_tail->next.reset();
         }
         _lru_head.reset();
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
        lru_node *prev;
        std::unique_ptr<lru_node> next;

        lru_node(const std::string &key, const std::string &value) :
              key(key), value(value), prev(nullptr), next(nullptr) {}
    };

    using index = std::map<std::reference_wrapper<const std::string>,
                    std::reference_wrapper<lru_node>, std::less<const std::string>>;

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;

    // Current storage size
    std::size_t _cur_size = 0;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    std::unique_ptr<lru_node> _lru_head;
    lru_node *_lru_tail;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    index _lru_index;

    bool _overflow(size_t new_size) const;
    void _insert_node(std::unique_ptr<lru_node> &node);
    void _get_up(lru_node *cur);
    bool _put_node(const std::string &key, const std::string &value);
    bool _set_node(const std::string &key, const std::string &value, index::iterator &it_find);

 };

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
