#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) 
{ 
//    std::cout << "In put\n";
    auto it = _lru_index.find(key);
    size_t data_size = key.length() + value.length();
    if (data_size > _max_size)
    {
        return false;
    }
    if (it == _lru_index.end())
    {
        if (_lru_index.empty())
        {
            _lru_head = std::move(std::unique_ptr<lru_node>(new lru_node{key, value, nullptr, nullptr}));
            _lru_index.insert({std::ref(_lru_head->key), std::ref(*_lru_head.get())});
            _lru_tail = _lru_head.get();
            _cur_size += data_size;
            return true;
        }
        else
        {   
            if (data_size + _cur_size <=  _max_size)
            { 
                _lru_tail->next = std::move(std::unique_ptr<lru_node>(new lru_node{key, value, _lru_tail, nullptr}));
                _lru_tail = _lru_tail->next.get();
                _lru_index.insert({std::ref(_lru_tail->key), std::ref(*_lru_tail)});
                _cur_size += data_size;  
                return true;  
            }    
            else
            {
                while(data_size + _cur_size > _max_size)
                {
                    if (_lru_head.get() != nullptr)
                    {
                        const std::string & str = _lru_head->key;
                        Delete(str);
                    }
                    else
                    {
                        break;                     
                    }
                                        
                }
                _lru_tail->next = std::move(std::unique_ptr<lru_node>(new lru_node{key, value, _lru_tail, nullptr}));
                _lru_tail = _lru_tail->next.get();
                _lru_index.insert({std::ref(_lru_tail->key), std::ref(*_lru_tail)});
                _cur_size += data_size;
                return true;
            }
            
        }   
    }
    else
    {
        lru_node & elem = it->second.get();
        _cur_size = _cur_size - sizeof(elem.value);
        elem.value = value;
       _cur_size = _cur_size + sizeof(elem.value);
       if (elem.next.get() == nullptr)
       {
           return true;
       }
       else if (elem.prev == nullptr)
       {
           _lru_tail->next = std::move(_lru_head);
           elem.prev = _lru_tail;
           _lru_tail = _lru_tail->next.get();
           _lru_head = std::move(elem.next);
           _lru_head->prev = nullptr;
           return true;
       }
       else
       {
           std::unique_ptr<lru_node> tmp = std::move(elem.prev->next);
           elem.next.get()->prev = elem.prev;
           elem.prev->next = std::move(elem.next);
           _lru_tail->next = std::move(tmp);
           elem.prev = _lru_tail;
           _lru_tail = _lru_tail->next.get();
           return true;
       }
       
    }
   
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    auto it = _lru_index.find(key);
    if (it == _lru_index.end())
    {
        return Put(key, value);
    }
    else
    {
        return false;
    }
    
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    auto it = _lru_index.find(key);
    if (it != _lru_index.end())
    {
        return Put(key, value);
    }
    else
    {
        return false;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
    auto it =_lru_index.find(key);
    if (it != _lru_index.end())
    {
        lru_node & elem = it->second.get();
        _cur_size = _cur_size - elem.value.length() - elem.key.length();
        _lru_index.erase(key);
       if (elem.next.get() == nullptr)
       {
           if (_lru_index.size() >1)
           {
               _lru_tail = _lru_tail->prev;
               _lru_tail->next = nullptr;
           }
           else
           {
               _lru_head = nullptr;
           }
           
       }
       else if (elem.prev == nullptr)
       {
           _lru_head = std::move(_lru_head->next);
           _lru_head->prev = nullptr;
       }
       else
       {
           std::unique_ptr<lru_node> tmp = std::move(elem.prev->next);
           elem.next.get()->prev = elem.prev;
           elem.prev->next = std::move(elem.next);
           tmp = nullptr;
       }
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
    if (_lru_index.size()>1)
    {
        if (&it->second.get() == _lru_head.get())
        {
            _lru_tail->next = std::move(_lru_head);
            _lru_tail->next.get()->prev = _lru_tail;
            _lru_tail = _lru_tail->next.get();
            _lru_head = std::move(_lru_tail->next);
            return true;        
        }
        if (&it->second.get() == _lru_tail)
        {
            return true;        
        }
        else
        {        
            lru_node * it_prev = it->second.get().prev;
            lru_node * it_next = it->second.get().next.get();
            auto tmp_it = std::move((*it_prev).next);
            (*it_prev).next = std::move(tmp_it.get()->next);
            (*it_next).prev = tmp_it->prev;
            tmp_it->prev = _lru_tail;
            _lru_tail->next = std::move(tmp_it);
            _lru_tail = _lru_tail->next.get();
        }
    }   
    return true;
}

} // namespace Backend
} // namespace Afina
