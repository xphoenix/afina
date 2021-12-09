#ifndef AFINA_STORAGE_STRIPED_LRU_H
#define AFINA_STORAGE_STRIPED_LRU_H

#include <map>
#include <mutex>
#include <string>
#include <functional>

#include "ThreadSafeSimpleLRU.h"

namespace Afina {
namespace Backend {

/**
 * # SimpleLRU thread safe version
 *
 *
 */
class StripedLRU : public Afina::Storage {
public:
    StripedLRU(size_t max_size = 1024, size_t amt_stripes = 4) {
        _max_size = max_size;
        for (int i = 0; i < amt_stripes; i = i + 1) {
            _mas_of_storages.emplace_back(std::unique_ptr<ThreadSafeSimplLRU>(new ThreadSafeSimplLRU(_max_size / amt_stripes)));
        }
    }
    static std::shared_ptr<StripedLRU> CheckParamsBuildStripedLRU(size_t max_size = 1024, size_t amt_stripes = 4){
        if (max_size/amt_stripes < 20*sizeof(char)){
            throw std::runtime_error("Too small size of each cache!");
        }
        return std::make_shared<StripedLRU>(max_size, amt_stripes); 
    }
    ~StripedLRU() {} //vector will delete all unique_ptrs, and unique_ptr will delete its object 
    

    // see SimpleLRU.h
    bool Put(const std::string &key, const std::string &value) override {
        size_t tek_hash = std::hash<std::string>{}(key);
        return _mas_of_storages[tek_hash % _mas_of_storages.size()]->Put(key,value);
    }

    // see SimpleLRU.h
    bool PutIfAbsent(const std::string &key, const std::string &value) override {
        size_t tek_hash = std::hash<std::string>{}(key);
        return _mas_of_storages[tek_hash % _mas_of_storages.size()]->PutIfAbsent(key,value);
    }

    // see SimpleLRU.h
    bool Set(const std::string &key, const std::string &value) override {
        size_t tek_hash = std::hash<std::string>{}(key);
        return _mas_of_storages[tek_hash % _mas_of_storages.size()]->Set(key,value);
    }

    // see SimpleLRU.h
    bool Delete(const std::string &key) override {
        size_t tek_hash = std::hash<std::string>{}(key);
        return _mas_of_storages[tek_hash % _mas_of_storages.size()]->Delete(key);
    }

    // see SimpleLRU.h
    bool Get(const std::string &key, std::string &value) override {
        size_t tek_hash = std::hash<std::string>{}(key);
        return _mas_of_storages[tek_hash % _mas_of_storages.size()]->Get(key,value);
    }

private:
    std::size_t _max_size;
    std::vector<std::unique_ptr<ThreadSafeSimplLRU>> _mas_of_storages; //doesn't work without ptrs
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_STRIPED_LRU_H