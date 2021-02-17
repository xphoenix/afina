#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// Delete the nodes until the _lru_cash_list is able to store object of size "size".
// Return false if _max_size <= size;
void SimpleLRU::cache_list_trim(size_t size) {
    while (_max_size - _current_size < size) {
         data_t deleted_pair = _lru_cash_list.back();
        _lru_index.erase(deleted_pair.first);
        _current_size -= deleted_pair.first.size() + deleted_pair.second.size();
        _lru_cash_list.pop_back();
    }
}


// Called only when pair {key, value} is not in cache 
bool SimpleLRU::ForcedPut(const std::string &key, const std::string &value) {
    cache_list_trim(key.size() + value.size());
    _lru_cash_list.push_front({key, value});
    _lru_index.emplace(std::make_pair(std::ref(_lru_cash_list.front().first), _lru_cash_list.begin()));
    _current_size += key.size() + value.size();
    return true;

}
// Called only when key is already in index
bool SimpleLRU::Set(lru_index_iterator it, const std::string &value){
    const std::string key = it->second->first;
    _current_size -= it->second->first.size() + it->second->second.size();
    _lru_cash_list.erase(it->second);
    _lru_index.erase(it);
    cache_list_trim(key.size() + value.size());
    _lru_cash_list.push_front({key, value});
    _lru_index.emplace(std::make_pair(std::ref(_lru_cash_list.front().first), _lru_cash_list.begin()));
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
    auto it = _lru_index.find(std::ref(key));
    if (it == _lru_index.end() || key.size() + value.size() > _max_size) {
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
        _lru_cash_list.erase(it->second);
        _lru_index.erase(it);
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
        _lru_cash_list.erase(it->second);
        _lru_index.erase(it);
        _lru_cash_list.push_front({key, value});
        // it->second = _lru_cash_list.begin(); //Qestion: when i changed second value of map iterator was use heap after free
        _lru_index.emplace(std::make_pair(std::ref(_lru_cash_list.front().first), _lru_cash_list.begin()));
        return true;
    }
}

} // namespace Backend
} // namespace Afina
