#include "SimpleLRU.h"
#include <iostream>
namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
  if (_lru_index.find(key) != _lru_index.end())
  {
    std::cout<<123<<std::endl;
    return Set(key,value);
  }
  if((key.size() + value.size()) > _max_size)
    return false;
  return push(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
  if (_lru_index.find(key) == _lru_index.end())
    return push(key,value);
  return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
  if (key.size()+value.size() > _max_size)
    return false;
  auto it = _lru_index.find(key);
  if (it == _lru_index.end())
    return false;
  if (_cursize - value.size() + it->second.get().value.size() > _max_size)
    return false;
  _cursize-=value.size() - it->second.get().value.size();
  if (!Delete(key))
    return false;
  return Put(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
  auto it = _lru_index.find(key);
  if (it == _lru_index.end())
    return false;
  _cursize-=key.size()+it->second.get().value.size();
  remove(it->second.get());
  _lru_index.erase(it);
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value)
{
  auto it = _lru_index.find(key);
  if (it == _lru_index.end())
  {
    return false;
  }
  value = it->second.get().value;
  if (!Delete(key))
    return false;
  return Put(key, value);
}

bool SimpleLRU::to_tail(lru_node& node, bool exists)
{
  if(exists)
    if (!remove(node))
      return false;

  if (_lru_index.size() == 1)
  {
    _lru_head  = (_lru_tail.get());
  }
  std::unique_ptr<lru_node> temp;
  _lru_tail->next = (&node);
  temp.swap(_lru_tail);
  node.prev.release();
  node.prev.swap(temp);
  _lru_tail.reset(&node);
  _lru_tail->next = nullptr;
  return true;
}
bool SimpleLRU::remove(lru_node& node)
{
  std::unique_ptr<lru_node> temp ;
  if (_lru_tail.get() == _lru_head)
  {
    _lru_tail.reset(nullptr);
  }
  if ((&node != _lru_tail.get()) && (&node != _lru_head))
  {
    node.prev->next = node.next;
    temp.swap(node.prev);
    node.next->prev.swap(temp);
  }
  else if (&node == _lru_head)
  {
    _lru_head = _lru_head->next;
    _lru_head->prev.reset(nullptr);
  }
  else
  {
    temp.swap(_lru_tail->prev);
    _lru_tail.swap(temp);
    if (_lru_tail.get() != _lru_head)
    {
      _lru_tail->next = nullptr;
    }
  }
  return true;
}

bool SimpleLRU::push(const std::string &key,const std::string &value)
{
  while(_cursize + key.size() + value.size() > _max_size)
      if (!Delete(_lru_head->key))
        return false;
  _cursize+= key.size() + value.size();
  auto nnode = new lru_node;
  nnode->key = key;
  nnode->value = value;
  if (_lru_tail == nullptr)
  {
    nnode->prev = nullptr;
    nnode->next =nullptr;
    _lru_tail.reset(nnode);
  }
  else
  {
    to_tail(*nnode, false);
  }
  _lru_index.insert(_lru_index.end(), std::make_pair(key, std::reference_wrapper<lru_node>(*nnode)));
  auto it = _lru_index.begin();
  for (; it!=_lru_index.end(); it++)
  {
    if (it->second.get().prev != nullptr)
      std::cout<<(it->second.get().prev->value)<<std::endl;
    else std::cout<<"0"<<std::endl;
    std::cout<<it->second.get().value<<std::endl;
    if (it->second.get().next != nullptr)
      std::cout<<(it->second.get().next->value)<<std::endl;
    else std::cout<<"0"<<std::endl;
    std::cout<<"-----"<<std::endl;
  }
  std::cout<<std::endl;
  return true;
}
} // namespace Backend
} // namespace Afina

std::string pad_space(const std::string &s, size_t length) {
    std::string result = s;
    result.resize(length, ' ');
    return result;
}

int main()
{
  Afina::Backend::SimpleLRU storage;
  std::cout<<(storage.Put("KEY1", "val1"))<<std::endl;
  std::cout<<(storage.Put("KEY2", "val2"))<<std::endl;
  std::string value;
  std::cout<<(storage.Get("KEY1", value))<<std::endl;
  std::cout<<(value == "val1")<<std::endl;
  std::cout<<(storage.Get("KEY2", value))<<std::endl;
  std::cout<<(value == "val2")<<std::endl;
}
