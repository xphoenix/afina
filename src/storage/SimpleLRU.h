#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <iostream>

#include <afina/Storage.h>

#define PRINTL std::cout << __LINE__ << std::endl;

namespace Afina {
namespace Backend {

class Lru_cash_list {
    public:
        // LRU cache node
        struct lru_node {
            const std::string key;
            std::string value;
            lru_node* prev;
            std::unique_ptr<lru_node> next;
            
            lru_node() = default;
            lru_node(std::string key_, std::string value_)
            : key(key_),
              value(value_)
              {}
        };

    private:

        std::unique_ptr<lru_node> _lru_head = nullptr;

    public:
        // Insert new element to begining of lru list
        void push_front(const std::string &key, const std::string &value);

        //  Return pointer to the last element
        lru_node* back() const;

        //  Return pointer to the first element
        lru_node* front() const;

        // Delete last elements from lru list
        void pop_back();

        // Delete first elements from lru list
        void pop_front();

        //Erase element on reference
        void erase(lru_node* node);
        
        // Clear list
        void reset();
};

/**
 * # Map based implementation
 * That is NOT thread safe implementaiton!!
 */
class SimpleLRU : public Afina::Storage {
public:
    SimpleLRU(size_t max_size = 1024) 
    : _max_size(max_size),
      _current_size(0) {}

    ~SimpleLRU() {
        _lru_index.clear();
        _lru_cash_list.reset(); // TODO: Here is stack overflow
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

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    // List owns all nodes
    Lru_cash_list _lru_cash_list;

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;
    
    // Number of bytes stored in cache 
    std::size_t _current_size = 0;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<Lru_cash_list::lru_node>, std::less<std::string>> _lru_index;

    bool cache_list_trim(size_t size);
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
