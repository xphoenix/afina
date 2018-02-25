#ifndef AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H
#define AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H

#include <map>
#include <mutex>
#include <string>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {

template<typename T>
class LRUListNode {
public:
    LRUListNode(): _value(T()), _next(nullptr), _prev(nullptr) {}
    LRUListNode(const T& value): _value(value), _next(nullptr), _prev(nullptr) {}

    // Set next item
    void Next(LRUListNode*) override;

    // Set previous item
    void Prev(LRUListNode*) override;

    // Delete links from neighbours
    void SoftDelete() override;

    // Delete links and destruct itself
    void HardDelete() override;

    // Getters for previous and next items
    LRUListNode* getPrev() override;
    LRUListNode* getNext() override;

    // Getter for value
    T Value() override;

    // Setter for value
    void Value(const T&) override;

    ~LRUListNode() {}

private:
    T _value;
    LRUListNode *_next, *_prev;
};

template<typename T>
class LRUList {
public:
    LRUList(): _tail(nullptr), _head(nullptr) {}

    // Add new item to the tail of list
    void Append(const T&) override;

    // Move node to the head of list
    void Up(LRUListNode<T>*) override;

    // Get head of list
    T Head() override;

    // Delete node from list
    void DeleteNode(LRUListNode<T>*) override;

    // Delete head from list
    void DeleteHead() override;

    ~LRUList() override;

private:
    LRUListNode<T> *_tail, *_head;
};

/**
 * # Map based implementation with global lock
 *
 *
 */
class MapBasedGlobalLockImpl : public Afina::Storage {
public:
    MapBasedGlobalLockImpl(size_t max_size = 1024) : _max_size(max_size) {}
    ~MapBasedGlobalLockImpl() {}

    // Implements Afina::Storage interface
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) const override;

    bool DeleteLRU() override;

    Insert(const std::string &key, const std::string &value) override;

private:
    size_t _max_size;

    struct Value;
    using map_iterator = std::map<std::string, Value>::iterator;
    std::map<std::string, Value> _backend;

    struct Value {
        std::pair<std::string, LRUListNode<map_iterator>* > value;
    };
    LRUList<map_iterator> _lru;

    // Implements Afina::Storage interface
    bool PutIfCould(std::string &key, std::string &value);
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H
