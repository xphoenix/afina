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
    class SimpleLRU: public Afina::Storage {
      public: SimpleLRU(size_t max_size = 1024): _max_size(max_size),
      _current_size(0) {}

      ~SimpleLRU() {
        _lru_index.clear();
        lru_node * del;
        del = _lru_head ? _lru_head -> prev : nullptr;

        while (del != _lru_head.get()) {
          lru_node * prev = del -> prev;
          prev -> next.reset();
          del = prev;
        }
        _lru_head.reset();
      }

      // Implements Afina::Storage interface
      bool Put(const std::string & key,
        const std::string & value) override;

      // Implements Afina::Storage interface
      bool PutIfAbsent(const std::string & key,
        const std::string & value) override;

      // Implements Afina::Storage interface
      bool Set(const std::string & key,
        const std::string & value) override;

      // Implements Afina::Storage interface
      bool Delete(const std::string & key) override;

      // Implements Afina::Storage interface
      bool Get(const std::string & key, std::string & value) override;

      private: struct lru_node;

      // LRU cache node
      struct lru_node {
        lru_node(const std::string & key,
          const std::string & value): key(key), value(value), prev(nullptr) {}
        const std::string key;
        std::string value;
        lru_node * prev;
        std::unique_ptr < lru_node > next;
      };

      // Maximum number of bytes could be stored in this cache.
      // i.e all (keys+values) must be less the _max_size
      std::size_t _max_size;
      std::size_t _current_size;

      // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
      // element that wasn't used for longest time. head->prev - the freshest element
      //
      // List owns all nodes
      std::unique_ptr < lru_node > _lru_head;
      // Index of nodes from list above, allows fast random access to elements by lru_node#key
      std::map < std::reference_wrapper <
      const std::string > , std::reference_wrapper < lru_node > , std::less < std::string >> _lru_index;
    };

  } // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H