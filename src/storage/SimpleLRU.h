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
    SimpleLRU(size_t max_size = 1024) : _max_size(max_size), _size(0), _lru_index(), _lru_head(), _lru_tail(nullptr) {}


    ~SimpleLRU() {
        _lru_index.clear();
        while(_lru_tail && _lru_tail!= _lru_head.get()) {
            _lru_tail = _lru_tail->prev;
            _lru_tail->next.reset();
        }
        _lru_head.reset(); // TODO: Here is stack overflow
    }

    std::size_t get_size ();


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
        std::string key;
        std::string value;
//        std::unique_ptr<lru_node> prev;
        lru_node* prev;
        std::unique_ptr<lru_node> next;
    };

    void add_key_value(const std::string &key, const std::string &value);
    void delete_from_tail();
    void change_value(lru_node &node, const std::string &value);
    void update_the_position(lru_node &node);
    bool is_overflow(const std::string &key, const std::string &value);

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be not greater than the _max_size
    std::size_t _max_size;

    //actual size of storage (size of key + value)
    std::size_t _size;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    std::unique_ptr<lru_node> _lru_head;

    lru_node *_lru_tail;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<std::string>> _lru_index;

};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
