#pragma once

#include <afina/Storage.h>
#include <assert.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
namespace Afina {
namespace Backend {

/**
 * # Map based implementation
 * That is NOT thread safe implementation!!
 */
class SimpleLRU : public Afina::Storage {
private:
    // state
    // LRU cache node
    using lru_node = struct lru_node {
        std::string const key;
        std::string value;
        lru_node *prev;
        std::unique_ptr<lru_node> next;
    };

    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be not greater than the _max_size
    std::size_t max_size;

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodes
    std::unique_ptr<lru_node> lru_head;

    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<std::string>>
        lru_index;

    std::size_t current_size;
    lru_node *last_node;

public:
    SimpleLRU(size_t _max_size = 1024) : max_size(_max_size), lru_head(nullptr), current_size(0) {}

    ~SimpleLRU() {
        lru_index.clear();
        while (static_cast<bool>(lru_head)) {
            lru_head = std::move(lru_head->next);
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
    void free_space(int64_t bytes_needed);
    void make_new_node(const std::string &key, const std::string &value);
    void move_in_head(lru_node &node);
};

} // namespace Backend
} // namespace Afina
