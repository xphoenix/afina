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
    _max_size(max_size),
    _cur_size(0)
    {
        lru_node *first_node = new lru_node("", "");
        lru_node *last_node = new lru_node("", "");
        _lru_head = std::unique_ptr<lru_node>(first_node);
        first_node->next = std::unique_ptr<lru_node>(last_node);
        first_node->prev = last_node;
        last_node->next = std::unique_ptr<lru_node>(nullptr);
        last_node->prev = first_node;
    }

    ~SimpleLRU() {
        _lru_index.clear();
        lru_node *last_node = _lru_head->prev;
        while(last_node->prev != _lru_head.get())
        {
            last_node = last_node->prev;
            last_node->next.reset(nullptr);
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
        lru_node(std::string _key, std::string _value): key(_key), value(_value){};
    };

    // Transfer node to the list's head
    void MoveNodeToHead(lru_node &node);

    // Create new LRU node
    lru_node *MakeLRUNode(const std::string &key, const std::string &value);

    // Delete nodes from LRU cache to free size
    void DropNodes(std::size_t size_to_release);

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;
    std::size_t _cur_size;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    std::unique_ptr<lru_node> _lru_head;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    using map_lru = std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<const std::string>>;
    map_lru _lru_index;
};
} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
