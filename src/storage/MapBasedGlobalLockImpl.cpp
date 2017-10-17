#include "MapBasedGlobalLockImpl.h"

#include <mutex>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
    std::unique_lock<std::mutex> guard(_lock);
    
    if (_elements.size()<_max_size){
        _elements[key] = value;
        return true;
    }
return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
    std::unique_lock<std::mutex> guard(_lock);
    
    auto it = _elements.find(key);
    if (it == _elements.end()){//еще нет ключа, итератор достиг конца
        _elements[key] =value;
        return true;
    }
    else return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
    std::unique_lock<std::mutex> guard(_lock);
    
    auto it = _elements.find(key);
    if (it != _elements.end()){//ключ нашёлся, итератор не достиг конца
        _elements[key]=value;
        return true;
    }
    else return false;
  
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) {
    std::unique_lock<std::mutex> guard(_lock);
    
    auto it = _elements.find(key);
    
    if (it != _elements.end()){
        _elements.erase (it);
    return true;
    }
    else return false;//не нашел такого
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
    std::unique_lock<std::mutex> guard(*const_cast<std::mutex *>(&_lock));
    auto it = _elements.find(key);
    
    if (it == _elements.end())
        return false;
    
    value = it->second;
    return true;
}
    



} // namespace Backend
} // namespace Afina
