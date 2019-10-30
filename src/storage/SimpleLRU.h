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
 * That is NOT thread safe implementation!!
 */
class SimpleLRU : public Afina::Storage {
public:
    SimpleLRU(size_t max_size = 1024) : _max_size(max_size), _cur_size(0) {}

    ~SimpleLRU() {
        _lru_index.clear();
        // _lru_head.reset(); // TODO: Here is stack overflow
        lru_node *p = _lru_head.get();
        if (p) {
            while (p->next) {
                p = p->next.get();
            }
            while (p->prev) {
                p = p->prev;
                p->next.reset();
            }
        }
        if (_lru_head) {
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
    using lru_node = struct lru_node {
        const std::string key;
        std::string value;
        lru_node *prev;
        std::unique_ptr<lru_node> next;

        lru_node(const std::string &_key, std::string _value)
            : key(_key), value(std::move(_value)), prev(nullptr), next(nullptr) {}
    };

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;
    std::size_t _cur_size;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    std::unique_ptr<lru_node> _lru_head = nullptr;
    //    lru_node *_lru_tail = nullptr;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<std::string>>
        _lru_index;
    using mapT =
        std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<std::string>>;

    void deb_print_list(lru_node *p) {
        // lru_node *p = _lru_head.get();
        while (p->next) {
            std::cerr << p->key << " : (" << p->value << ");        ";
            if (p->prev == nullptr) {
                std::cerr << "NULLPTR\n";
            }
            p = p->next.get();
        }
        std::cerr << p->key << " : (" << p->value << ");        ";
        if (p->prev == nullptr) {
            std::cerr << "NULLPTR\n";
        }
        std::cerr << std::endl;
    }

    void free_memmory_for_node(const std::string &key, const std::string &value);
    bool _PutIfAbsent(mapT::iterator it, const std::string &key, const std::string &value);
    bool _Set(mapT::iterator it, const std::string &key, const std::string &value);
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
