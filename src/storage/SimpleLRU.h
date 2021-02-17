#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <iostream>
#include <list>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {
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
        _lru_cashe_list.clear();
         // TODO: Here is stack overflow
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

    using data_t = std::pair<std::string, std::string>;
    using lru_list_iterator = std::list<std::pair<const std::string, std::string>>::iterator;
    using lru_index_iterator = std::map<std::reference_wrapper<const std::string>, lru_list_iterator, std::less<std::string>>::iterator;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    std::list<std::pair<const std::string, std::string>> _lru_cashe_list; 

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    std::size_t _max_size;
    
    // Number of bytes stored in cache 
    std::size_t _current_size = 0;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, lru_list_iterator, std::less<std::string>> _lru_index;

    void CacheListTrim(size_t size);

    bool ForcedPut(const std::string &key, const std::string &value);
    bool Set(lru_index_iterator it, const std::string &value);
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
