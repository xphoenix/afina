#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <iostream>
#include </home/eles/Public/afina/include/afina/Storage.h>
namespace Afina {
namespace Backend {

/**
 * # Map based implementation
 * That is NOT thread safe implementaiton!!
 */
class SimpleLRU : public Afina::Storage {
public:
    SimpleLRU(size_t max_size = 1024) : _max_size(max_size), _cursize(0)
    {
      _lru_head = nullptr;
      _lru_tail = (nullptr);
    }

    ~SimpleLRU()
    {
       if (_lru_tail == nullptr)
         return;
       _lru_index.clear();
       while(_lru_tail.get() != _lru_head)
         remove(*_lru_tail);
        _lru_head = nullptr;
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
        std::string key;
        std::string value;
        std::unique_ptr<lru_node> prev;
        lru_node* next;
    };

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;
    std::size_t _cursize;
    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    lru_node* _lru_head;
    std::unique_ptr<lru_node> _lru_tail;
    bool to_tail(lru_node& node, bool exists);
    bool remove(lru_node& node);
    bool push(const std::string &key,const std::string &value);
    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::string, std::reference_wrapper<lru_node>, std::less<std::string>> _lru_index;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
