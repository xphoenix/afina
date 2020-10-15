#include "ThreadSafeSimpleLRU.h"

namespace Afina {
namespace Backend {

  //add new element to the storage
  bool ThreadSafeSimpleLRU::putElement(const std::string &key, const std::string &value) {

    std::size_t newsz = key.size() + value.size();
    if (newsz > _max_size) {
      return false; //not enough space
    }

    while (newsz + _cur_size > _max_size) {
      deleteNode(std::ref(*_lru_head)); //delete least recently used element
    }

    auto new_node = new lru_node { key, 
                                   value,
                                   _lru_tail,
                                   nullptr };

    auto node_ptr = std::unique_ptr<lru_node>(new_node);

    if (_lru_head == nullptr) { //empty storage
      //initiate storage

      _lru_head = std::move(node_ptr);

      } else { //add new element to existing storage
        _lru_tail->next = std::move(node_ptr);
      }

    _lru_tail = new_node;

    _cur_size += newsz;
    _lru_index.emplace(std::cref(_lru_tail->key), std::ref(*new_node));

    return true;
  }

  //update the value of an exisiting element of the storage
  bool ThreadSafeSimpleLRU::updateValue(ThreadSafeSimpleLRU::lru_node &node, const std::string &value) {
    std::size_t oldsz = node.value.size();
    std::size_t newsz = value.size();

    if (node.key.size() + newsz > _max_size) {
      return false;
    }

    toTail(node);

    while (_cur_size - oldsz + newsz > _max_size) {
      deleteNode(std::ref(*_lru_head));
    }

    _cur_size -= oldsz;
    _cur_size += newsz; 

    node.value = value;

    return true;
  }

  //delete existing node
  void ThreadSafeSimpleLRU::deleteNode(ThreadSafeSimpleLRU::lru_node &node) {

    _cur_size -= node.key.size() + node.value.size();
    _lru_index.erase(node.key);

    if (_lru_head.get() == _lru_tail) {
      //there is only one element in the storage, which is "node"
      //(deleteNode is only called for existing elements);
      //deleting it 

      _lru_head = nullptr;
      //memory released during unique_ptr destruction
      
      _lru_tail = nullptr;
      _cur_size = 0;

    } else if (node.prev == nullptr) { //deleting head
      _lru_head = std::move(_lru_head->next); //memory released during unique_ptr destruction
      _lru_head->prev = nullptr;
    
    } else if (node.next == nullptr) { //deleting tail
      _lru_tail = _lru_tail->prev;
            _lru_tail->next = nullptr; //memory released during unique_ptr destruction
    
    } else { //deleting some other element 
      lru_node *prev = node.prev;
      prev->next = std::move(node.next); //memory released during unique_ptr destruction
      prev->next->prev = prev;
    }
  }

  //move existing node to the tail of the deletion queue
  //(make it the most recently used element)
  void ThreadSafeSimpleLRU::toTail(ThreadSafeSimpleLRU::lru_node &node) {
    if (node.next != nullptr) { //not already tail

      auto ptr = node.next->prev;
      ptr->next->prev = ptr->prev;
            
      if (ptr->prev == nullptr) { //moving head element to tail
        _lru_tail->next = std::move(_lru_head);
        _lru_head = std::move(ptr->next);
      } else { 
        _lru_tail->next = std::move(ptr->prev->next);
        ptr->prev->next = std::move(ptr->next);
      }
            
      ptr->next = nullptr;
      ptr->prev = _lru_tail;
      _lru_tail = ptr;      
    }
  }

  // See MapBasedGlobalLockImpl.h
  bool ThreadSafeSimpleLRU::Put(const std::string &key, const std::string &value) {
        
  	std::lock_guard<std::mutex> lock(storage_available);
        
    auto found = _lru_index.find(key);
    if (found == _lru_index.end()) {
      return putElement(key, value);
    } else {
      ThreadSafeSimpleLRU::lru_node &node = found->second.get();
      return updateValue(node, value);
      }
    }

  // See MapBasedGlobalLockImpl.h
  bool ThreadSafeSimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
        
  	std::lock_guard<std::mutex> lock(storage_available);

    auto found = _lru_index.find(key);
    if (found == _lru_index.end()) {
      return putElement(key, value); 
    } else {
      return false; 
    }
  }

  // See MapBasedGlobalLockImpl.h
  bool ThreadSafeSimpleLRU::Set(const std::string &key, const std::string &value) {

  	std::lock_guard<std::mutex> lock(storage_available);

    auto found = _lru_index.find(key);
    if (found == _lru_index.end()) { 
      return false; //not found
    }

    ThreadSafeSimpleLRU::lru_node &node = found->second.get();
    return updateValue(node, value);
  }

  // See MapBasedGlobalLockImpl.h
  bool ThreadSafeSimpleLRU::Delete(const std::string &key) {
        
  	std::lock_guard<std::mutex> lock(storage_available);

    auto found = _lru_index.find(key);
    if (found == _lru_index.end()) {
      return false; //not found
    }

    ThreadSafeSimpleLRU::lru_node &node = found->second.get();

    deleteNode(node);

    return true;
  }

  // See MapBasedGlobalLockImpl.h
  bool ThreadSafeSimpleLRU::Get(const std::string &key, std::string &value) {

  	std::lock_guard<std::mutex> lock(storage_available);

    auto found = _lru_index.find(key);
    if (found == _lru_index.end()) { 
      return false; //not found
    }

    ThreadSafeSimpleLRU::lru_node &node = found->second.get();
    value = node.value;
    toTail(node); //this element is now the most recently used
                  //moving it to the end of the deletion queue

    return true;
  }

} // namespace Backend
} // namespace Afina
