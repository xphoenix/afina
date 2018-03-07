#ifndef AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H
#define AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H

#include <unordered_map>
#include <mutex>
#include <string>
#include <functional>


#include <afina/Storage.h>

namespace Afina {
namespace Backend {

template<typename T>
class LRUListNode {
public:
    LRUListNode(): _value(T()), _next(nullptr), _prev(nullptr) {}
    LRUListNode(const T& value): _value(value), _next(nullptr), _prev(nullptr) {}

    // Set next item
    void Next(LRUListNode*);

    // Set previous item
    void Prev(LRUListNode*);

    // Delete links from neighbours
    void Delete();

    // Getters for previous and next items
    LRUListNode* getPrev();
    LRUListNode* getNext();

    // Getter for value
    T Value();

    // Setter for value
    void Value(const T&);

    ~LRUListNode();

private:
    T _value;
    LRUListNode *_next, *_prev;
};

template<typename T>
class LRUList {
public:
    LRUList(): _tail(nullptr), _head(nullptr) {}

    // Add new item to the tail of list
    void Append(const T&) ;

    // Move node to the head of list
    void Up(LRUListNode<T>*);

    // Get head of list
    LRUListNode<T>* Head();

    // Get tail of list
    LRUListNode<T>* Tail();

    // Delete node from list
    void DeleteNode(LRUListNode<T>*);

    // Delete head from list
    void DeleteHead();

    ~LRUList();

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
    MapBasedGlobalLockImpl(size_t max_size = 1024) : _max_size(max_size), _usage_size(0) {}
    ~MapBasedGlobalLockImpl();

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


private:
    using string_reference = std::reference_wrapper<std::string>;
    using string_pointer = std::string*;
    using unordered_map_type = std::unordered_map<
            string_reference,
            std::pair<string_pointer, LRUListNode<string_reference>* >,
            std::hash<std::string>,
            std::equal_to<std::string>
    >;

    size_t _max_size;
    size_t _usage_size;
    
    mutable std::mutex _global_lock;

    unordered_map_type _backend;

    mutable LRUList<string_reference> _lru;

    // Update existsting element in storage
    bool _Update(const std::string &key, const std::string &value, unordered_map_type::iterator &elem_iter);

    // Insert new element in storage
    bool _Insert(const std::string &key, const std::string &value);

    // Delete Last Recently Used
    bool _DeleteLRU();

    // Clear storage data if need
    void _ClearUsageData(const std::string &, const std::string &, unordered_map_type::iterator &);
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H
