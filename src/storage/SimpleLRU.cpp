#include "SimpleLRU.h"

#include <stdexcept>

namespace Afina {
namespace Backend {

void SimpleLRU::NodeTransfer(lru_node &node)
{
    if(node.prev == nullptr)
        return; // first node in list

    _lru_head->prev = &node;
    if(node.next)
        node.next->prev = node.prev;

    std::unique_ptr<lru_node> tmp_ptr = std::move(node.prev->next);
    node.prev->next = std::move(node.next);
    node.prev = nullptr;
    node.next = std::move(_lru_head);
    _lru_head = std::move(tmp_ptr);
}

SimpleLRU::lru_node *SimpleLRU::MakeNode(const std::string &key, const std::string &value)
{
    lru_node *new_node = new lru_node(key);
    if(_lru_head)
        _lru_head->prev = new_node;
    // new_node->key = key;
    new_node->value = value;
    new_node->prev = nullptr;
    new_node->next = std::move(_lru_head);
    _lru_head = std::unique_ptr<lru_node>(new_node);
    return new_node;
}

void SimpleLRU::DropNodes(std::size_t size)
{
  lru_node *last_node = _lru_head.get();
  while (_lru_head && (_cur_size + size > _max_size))
  {
      while(last_node->next.get() != nullptr)
          last_node = last_node->next.get();

      _cur_size -= last_node->key.size();
      _cur_size -= last_node->value.size();
      _lru_index.erase(last_node->key);

      last_node = last_node->prev;
      if (last_node == nullptr)
          _lru_head.reset(nullptr);
      else
          last_node->next.reset(nullptr);
  }
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
    if (key.size() + value.size() > _max_size)
        throw std::runtime_error("Limit size exceeded");

    auto map_it = _lru_index.find(key);
    std::size_t put_size = value.size();
    std::size_t out_size = 0;

    if (map_it != _lru_index.end())
    {
        NodeTransfer(map_it->second.get());
        out_size = map_it->second.get().value.size();
    }
    else
        put_size += key.size();
    // if necessary will be free size in cache
    DropNodes(put_size - out_size);

    _cur_size += put_size;
    _cur_size -= out_size;
    if (map_it != _lru_index.end())
        map_it->second.get().value = value;
    else
    {
        lru_node *node = MakeNode(key, value);
        _lru_index.insert(std::make_pair(std::ref(node->key), std::ref(*node)));
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    std::size_t put_size = value.size() + key.size();
    if (put_size > _max_size)
        throw std::runtime_error("Limit size exceeded");
        // return false;
    if(_lru_index.find(key) != _lru_index.end())
        return false;
    // if necessary will be free size in cache
    DropNodes(put_size);

    _cur_size += put_size;
    lru_node *node = MakeNode(key, value);
    _lru_index.insert(std::make_pair(std::ref(node->key), std::ref(*node)));
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    std::size_t put_size = value.size() + key.size();
    std::size_t out_size = 0;
    auto map_it = _lru_index.find(key);
    if (put_size > _max_size)
        throw std::runtime_error("Limit size exceeded");
        // return false;
    if (map_it == _lru_index.end())
        return false;

    NodeTransfer(map_it->second.get());
    out_size = map_it->second.get().value.size();
    // if necessary will be free size in cache
    DropNodes(put_size - out_size);

    _cur_size += put_size;
    _cur_size -= out_size;
    map_it->second.get().value = value;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
    auto map_it = _lru_index.find(key);
    if (map_it == _lru_index.end())
        return false;

    lru_node &node_to_drop = map_it->second.get();
    _cur_size -= key.size() + node_to_drop.value.size();

    _lru_index.erase(map_it);

    if(node_to_drop.next)
        node_to_drop.next->prev = node_to_drop.prev;

    if(node_to_drop.prev == nullptr)
        _lru_head = std::move(node_to_drop.next);
    else
        node_to_drop.prev->next = std::move(node_to_drop.next);

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value)
{
    auto map_it = _lru_index.find(key);
    if (map_it == _lru_index.end())
        return false;

    value = map_it->second.get().value;
    return true;
}

} // namespace Backend
} // namespace Afina
