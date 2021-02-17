#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// Delete the nodes until the _lru_cashe_list is able to store object of size "size".
// Return false if _max_size <= size;
void SimpleLRU::CacheListTrim(size_t size) {
    while (_max_size - _current_size < size) {
         const data_t &deleted_pair = _lru_cashe_list.back();
        _lru_index.erase(deleted_pair.first);
        _current_size -= deleted_pair.first.size() + deleted_pair.second.size();
        _lru_cashe_list.pop_back();
    }
}


// Called only when pair {key, value} is not in cache 
bool SimpleLRU::ForcedPut(const std::string &key, const std::string &value) {
    CacheListTrim(key.size() + value.size());
    _lru_cashe_list.push_front({key, value});
    _lru_index.emplace(std::make_pair(std::ref(_lru_cashe_list.front().first), _lru_cashe_list.begin()));
    _current_size += key.size() + value.size();
    return true;

}
// Called only when key is already in index
bool SimpleLRU::Set(lru_index_iterator it, const std::string &value) {
    _lru_cashe_list.splice(_lru_cashe_list.begin(), _lru_cashe_list, it->second); //insert it->second to _lru_cashe_list.begin()
    
    if (value.size() > it->second->second.size())
        CacheListTrim(value.size() - it->second->second.size());

    it->second->second = value;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        return ForcedPut(key, value);
    } else {
        Set(it, value);
    }

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    auto it = _lru_index.find(key); 
    if (it == _lru_index.end()){
        return ForcedPut(key, value);
    } else {
        return false;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size)
        return false;

    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        return false;
    } else {
        return Set(it, value);
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        return false;
    } else {
        lru_list_iterator list_it = it->second;
        _lru_index.erase(it);
        _lru_cashe_list.erase(list_it);
        return true;    
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end()) {
        return false;
    } else {
        value = it->second->second; 
        _lru_cashe_list.splice(_lru_cashe_list.begin(), _lru_cashe_list, it->second); //insert it->second to _lru_cashe_list.begin()
        return true;
    }
}

} // namespace Backend
} // namespace Afina
