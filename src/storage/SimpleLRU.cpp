#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

void SimpleLRU::MoveNodeToHead(lru_node &node)
{
    if(node.prev == _lru_head.get())
    {
        return; // first usefull node in list
    }
    node.next->prev = node.prev;
    std::unique_ptr<lru_node> tmp_ptr = std::move(node.prev->next);
    node.prev->next = std::move(node.next);
    _lru_head->next->prev = tmp_ptr.get();
    node.prev = _lru_head.get();
    node.next = std::move(_lru_head->next);
    _lru_head->next = std::move(tmp_ptr);
}

SimpleLRU::lru_node *SimpleLRU::MakeLRUNode(const std::string &key, const std::string &value)
{
    lru_node *new_node = new lru_node(key, value);
    new_node->prev = _lru_head.get();
    new_node->next = std::move(_lru_head->next);
    new_node->next->prev = new_node;
    _lru_head->next = std::unique_ptr<lru_node>(new_node);
    return new_node;
}

void SimpleLRU::DropNodes(std::size_t size)
{
  lru_node *last_node = _lru_head->prev->prev;
  while ((_lru_head.get() != last_node) && (_cur_size + size > _max_size))
  {
      _cur_size -= last_node->key.size();
      _cur_size -= last_node->value.size();
      _lru_index.erase(last_node->key);

      last_node->next->prev = last_node->prev;
      last_node = last_node->prev;
      last_node->next = std::move(last_node->next->next);
  }
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
    if (key.size() + value.size() > _max_size)
    {
        return false;
    }

    auto map_it = _lru_index.find(key);
    std::size_t put_size = value.size();
    std::size_t out_size = 0;

    if (map_it != _lru_index.end())
    {
        MoveNodeToHead(map_it->second.get());
        out_size = map_it->second.get().value.size();
    }
    else
    {
        put_size += key.size();
    }
    // if necessary will be free size in cache
    DropNodes(put_size - out_size);

    _cur_size += put_size;
    _cur_size -= out_size;
    if (map_it != _lru_index.end())
    {
        map_it->second.get().value = value;
    }
    else
    {
        lru_node *node = MakeLRUNode(key, value);
        _lru_index.insert(std::make_pair(std::ref(node->key), std::ref(*node)));
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    std::size_t put_size = value.size() + key.size();
    if (put_size > _max_size)
    {
        return false;
    }
    if(_lru_index.find(key) != _lru_index.end())
    {
        return false;
    }
    // if necessary will be free size in cache
    DropNodes(put_size);

    _cur_size += put_size;
    lru_node *node = MakeLRUNode(key, value);
    _lru_index.insert(std::make_pair(std::ref(node->key), std::ref(*node)));
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    std::size_t put_size = value.size() + key.size();
    std::size_t out_size = 0;
    if (put_size > _max_size)
    {
        return false;
    }
    auto map_it = _lru_index.find(key);
    if (map_it == _lru_index.end())
    {
        return false;
    }

    MoveNodeToHead(map_it->second.get());
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
    {
        return false;
    }

    lru_node &node_to_drop = map_it->second.get();
    _cur_size -= key.size() + node_to_drop.value.size();

    _lru_index.erase(map_it);
    node_to_drop.next->prev = node_to_drop.prev;
    node_to_drop.prev->next = std::move(node_to_drop.next);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value)
{
    auto map_it = _lru_index.find(key);
    if (map_it == _lru_index.end())
    {
        return false;
    }
    value = map_it->second.get().value;
    return true;
}

} // namespace Backend
} // namespace Afina
