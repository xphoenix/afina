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
    SimpleLRU(size_t max_size = 1024) :
            _size(0),
            _max_size(max_size),
            _lru_head(nullptr),
            _lru_tail(nullptr)
     {
     }

    ~SimpleLRU() {
        this->_lru_index.clear();
        while (this->_lru_head) {
            std::unique_ptr<lru_node> next = std::move(this->_lru_head->next);
            this->_lru_head.reset(nullptr);
            this->_lru_head = std::move(next);
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
    };

    // Current size
    std::size_t _size;
    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be not greater than the _max_size
    std::size_t _max_size;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    std::unique_ptr<lru_node> _lru_head;
    lru_node *_lru_tail;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<const std::string>> _lru_index;

    bool _Put(const std::string &key, const std::string &value);
    bool _UpdateValue(lru_node &node, const std::string &value);
    void _DeleteNode(lru_node &node);
    bool _MoveNode(lru_node &node);

};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
