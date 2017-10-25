#include "MapBasedGlobalLockImpl.h"

#include <mutex>

namespace Afina {
    namespace Backend {
        
        // See MapBasedGlobalLockImpl.h
        bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
            std::unique_lock<std::mutex> guard(_lock);
            if (_backend.size()<_max_size){
                _backend[key]=value;
                _order.push_front(key);
                return true;
            }
            else{
                std::string temp_key = _order.back();
                _order.pop_back();
                _backend.erase(temp_key);
                _order.push_front(key);
                _backend[key] = value;
                return true;
            }
            
        }
        
        bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
            std::unique_lock<std::mutex> guard(_lock);
            if ( _backend.find(key) == _backend.end() ){
                return MapBasedGlobalLockImpl::Put(key, value);
            }
            else return false;
        }
        
        
        bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
            std::unique_lock<std::mutex> guard(_lock);
            if ( _backend.find(key) != _backend.end() ){
                _backend[key] = value;
                return true;
            }
            else return false;
        }
        
        bool MapBasedGlobalLockImpl::Delete(const std::string &key) {
            std::unique_lock<std::mutex> guard(_lock);
            if ( _backend.find(key) != _backend.end()) {
                _backend.erase(key);
                _order.remove(key);
                return true;
            }
            else return false;
        }
        
        bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
            std::unique_lock<std::mutex> guard(*const_cast<std::mutex *>(&_lock));
            if ( _backend.find(key) != _backend.end()) {
                value = _backend.at(key);
                return true;
            }
            else return false;
        }
        
        
        
        
    } // namespace Backend
} // namespace Afina
