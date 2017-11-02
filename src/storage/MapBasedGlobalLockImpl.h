#ifndef AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H
#define AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H

#include <map>
#include <mutex>
#include <string>
#include <queue>
#include <afina/Storage.h>
#include <list>
#include <algorithm>


namespace Afina {
    namespace Backend {
        using std::find;
        /**
         * # Map based implementation with global lock
         *
         *
         */
        class MapBasedGlobalLockImpl : public Afina::Storage {
        public:
            MapBasedGlobalLockImpl(size_t max_size = 1024) : _max_size(max_size) {}
            ~MapBasedGlobalLockImpl() {}
            
            // Implements Afina::Storage interface
            bool Put(const std::string &key, const std::string &value) override;
            
            // Implements Afina::Storage interface
            bool PutIfAbsent(const std::string &key, const std::string &value) override;
            
            // Implements Afina::Storage interface
            bool Set(const std::string &key, const std::string &value) override;
            
            // Implements Afina::Storage interface
            bool Delete(const std::string &key) override;
            
            // Implements Afina::Storage interface
            bool Get(const std::string &key, std::string &value) const override;
            
        private:
            std::mutex _lock;
            
            size_t _max_size;
            
            std::list <std::string> age;
            std::map<std::string, std::string> map;
            
            
        };
        
    } // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H
