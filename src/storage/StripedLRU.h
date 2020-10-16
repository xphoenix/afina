#ifndef AFINA_STORAGE_STRIPED_LRU_H
#define AFINA_STORAGE_STRIPED_LRU_H

#include <afina/Storage.h>
#include "SimpleLRU.h"

#include <vector>
#include <functional>

namespace Afina {
namespace Backend {

class StripedLRU : public Afina::Storage {
    std::hash<std::string> hash_func;
    std::vector<std::unique_ptr<SimpleLRU>> shards;
public:
    
    StripedLRU(std::size_t n_shards = 10, std::size_t memory_limit = 1024 * 1024 * 10) {

    	std::size_t shard_limit = memory_limit / n_shards;

    	if (shard_limit < 1024 * 1024) { //less than 1 MB
			throw std::runtime_error("Too few memory for each shard.");	
    	}

        for (size_t i = 0; i < n_shards; i++) {
            shards.push_back(std::unique_ptr<SimpleLRU> (new SimpleLRU(shard_limit)));
        }
    }

    ~StripedLRU() {}

    bool Put(const std::string &key, const std::string &value) override ;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override ;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) override ;

};
} //namespace Backend
} //namespace Afina

#endif //AFINA_STORAGE_STRIPED_LRU_H
