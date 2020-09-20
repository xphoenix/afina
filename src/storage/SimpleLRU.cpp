#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) 
{ 
    std::cout << "\n\nIn put";
    bool in_map = (_lru_index.find(std::reference_wrapper<const std::string>(key)) == _lru_index.end());
    std::cout << "\n\n\n нет в таблице" << in_map << "\n\n\n";
    size_t data_size = sizeof(key) + sizeof(value);
    std::cout << "\n\n\n" << data_size << "\n\n\n";
    if (in_map)
    {
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
        if (!_lru_index.empty())
        {
            lru_node& tmp = _lru_index.rbegin()->second.get();
            tmp.next = std::move(std::unique_ptr<lru_node>(new lru_node{key, value, &tmp, nullptr}));
            _lru_index.insert({key, *tmp.next.get()});
        }
        else
        {       
            _lru_head = std::move(std::unique_ptr<lru_node>(new lru_node{key, value, nullptr, nullptr})); 
            _lru_index.insert(std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>>(key, *_lru_head));  
            _cur_size += data_size;
        }
        std::cout << "\n" << _lru_index.find(key)->second.get().value << "\n";
        return true;
    }
    auto tmp = _lru_index.find(key);
    tmp->second.get().value = value;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    bool in_map = (_lru_index.find(key) == _lru_index.end());
    size_t data_size = sizeof(key) + sizeof(value);
    if (in_map)
    {
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
        tmp.next = std::move(std::unique_ptr<lru_node>(new lru_node{key, value, &tmp, nullptr}));
        _lru_index.insert({key, *tmp.next});
        _cur_size += data_size;
        return true;
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    bool in_map = (_lru_index.find(key) == _lru_index.end());
    size_t data_size = sizeof(key) + sizeof(value);
    if (in_map)
    {
        return false;
    }
    auto tmp = _lru_index.find(key);
    tmp->second.get().value = value;
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
    std::cout << "\n GET \n";
    std::string str = "KEY1";
    auto it =_lru_index.begin();
//    auto it =_lru_index.find(key);
    if (it == _lru_index.end())
    {
        return false;
    }
    value = it->second.get().value;
    if (_lru_index.size()>1)
    {
        auto it_temp = it--;
        it_temp->second.get().next = std::move(it->second.get().next); 
        lru_node& head = _lru_index.rbegin()->second.get();
        it->second.get().prev = &head;
    }
    return true;
}

} // namespace Backend
} // namespace Afina
