#include "MapBasedGlobalLockImpl.h"
#include <iostream>

#include <mutex>

namespace Afina {
    namespace Backend {
        
        // See MapBasedGlobalLockImpl.h
        
        bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
            
            std::map<std::string, std::string>::iterator it = this->map.find(key);
            if (it == this->map.end()) {
                if (this->map.size() < this->_max_size) {
                    this->map.insert(std::make_pair(key, value));
                    
                } else {
                    this->map.erase(*age.begin());
                    this->age.erase(this -> age.begin());
                    this->map.insert(std::make_pair(key, value));
                }
                
            }else {
                (it -> second) = value;
                this->age.erase(find(age.begin(), age.end(), key));
            }
            this -> age.push_back(key);
            
            return true;
        }
        
        // See MapBasedGlobalLockImpl.h
        bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
            
            std::map<std::string, std::string>::iterator it = this->map.find(key);
            if (it == this->map.end()) {
                if (this->map.size() < this->_max_size) {
                    this->map.insert(std::make_pair(key, value));
                } else {
                    
                    this->map.erase(*age.begin());
                    this->age.erase(this -> age.begin());
                    this->map.insert(std::make_pair(key, value));
                }
                this -> age.push_back(key);
                return true;
            }
            
            return false;
        }
        
        // See MapBasedGlobalLockImpl.h
        bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
            std::map<std::string, std::string>::iterator it = this->map.find(key);
            if (it != this->map.end()) {
                it -> second = value;
                
                this-> age.erase(find(age.begin(), age.end(), key));
                this -> age.push_back(key);
                
            }else {
                return false;
            }
            return true;
        }
        
        // See MapBasedGlobalLockImpl.h
        bool MapBasedGlobalLockImpl::Delete(const std::string &key) {
            std::map<std::string, std::string>::iterator it = this->map.find(key);
            if (it == this->map.end()) {
                return false;
            }
            this->map.erase(it);
            this->age.erase(find(age.begin(), age.end(), key));
            
            return true;
        }
        
        // See MapBasedGlobalLockImpl.h
        bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
            std::map<std::string, std::string>::const_iterator it = this->map.find(key);
            if (it == this->map.end()) {
                return false;
            } else {
                value = (it->second);
                
                return true;
            }
        }
        
    } // namespace Backend
} // namespace Afina
