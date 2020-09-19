#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) 
{ 
    size_t data_size = sizeof(key) + sizeof(value);
    if (data_size > _max_size)
    {
        return false;
    }
    if (data_size + _cur_size > _max_size)
    {
        while (data_size + _cur_size > _max_size)
        {
            _cur_size -= sizeof(_lru_head->key) + sizeof(_lru_head->value);
            _lru_index.erase(_lru_head->key);
            _lru_head = std::move(_lru_head->next);
            delete _lru_head->prev;
            _lru_head->prev = nullptr;
        }
    }
    lru_node& tmp = _lru_index.rbegin()->second.get();
    tmp.next = std::unique_ptr<lru_node>(new lru_node{key, value, &tmp, nullptr});
    _lru_index.insert({key, *tmp.next});
    _cur_size += data_size;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    size_t data_size = sizeof(key) + sizeof(value);
    if ((data_size > _max_size) || (_lru_index.find(key) != _lru_index.end()))
    {
        return false;
    }
    if (data_size + _cur_size > _max_size)
    {
        while (data_size + _cur_size > _max_size)
        {
            _cur_size -= sizeof(_lru_head->key) + sizeof(_lru_head->value);
            _lru_index.erase(_lru_head->key);
            _lru_head = std::move(_lru_head->next);
            delete _lru_head->prev;
            _lru_head->prev = nullptr;
        }
    }
    lru_node& tmp = _lru_index.rbegin()->second.get();
    tmp.next = std::unique_ptr<lru_node>(new lru_node{key, value, &tmp, nullptr});
    _lru_index.insert({key, *tmp.next});
    _cur_size += data_size;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    size_t data_size = sizeof(key) + sizeof(value);
    if ((data_size > _max_size) || (_lru_index.find(key) == _lru_index.end()))
    {
        return false;
    }
    if (data_size + _cur_size > _max_size)
    {
        while (data_size + _cur_size > _max_size)
        {
            _cur_size -= sizeof(_lru_head->key) + sizeof(_lru_head->value);
            _lru_index.erase(_lru_head->key);
            _lru_head = std::move(_lru_head->next);
            delete _lru_head->prev;
            _lru_head->prev = nullptr;
        }
    }
    lru_node& tmp = _lru_index.rbegin()->second.get();
    tmp.next = std::unique_ptr<lru_node>(new lru_node{key, value, &tmp, nullptr});
    _lru_index.insert({key, *tmp.next});
    _cur_size += data_size;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
    auto it =_lru_index.find(key);
    if (it != _lru_index.end())
    {
        lru_node& temp = it->second.get();
        auto it_temp = it--;
        auto ref_for = std::move(temp.next);
        delete it_temp->second.get().next.get();
        it_temp->second.get().next = std::move(ref_for);
        _lru_index.erase(key);
        return true;
    }
    else
    {
        return false;    
    }
    
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value)
{
    auto it =_lru_index.find(key);
    if (it == _lru_index.end())
    {
        return false;
    }
    value = it->second.get().value;
    auto it_temp = it--;
    it_temp->second.get().next = std::move(it->second.get().next); 
    lru_node& head = _lru_index.rbegin()->second.get();
    it->second.get().prev = &head;
    return true;
}

} // namespace Backend
} // namespace Afina
