#include "StripedLRU.h"

namespace Afina {
namespace Backend {

bool StripedLRU::Put(const std::string &key, const std::string &value)  {
    return shards[hash_func(key) % shards.size() ]->Put(key, value);
}

// Implements Afina::Storage interface
bool StripedLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    return shards[hash_func(key) % shards.size() ]->PutIfAbsent(key, value);
}

// Implements Afina::Storage interface
bool StripedLRU::Set(const std::string &key, const std::string &value) {
    return shards[hash_func(key) % shards.size() ]->Set(key, value);
}

// Implements Afina::Storage interface
bool StripedLRU::Delete(const std::string &key)  {
    return shards[hash_func(key) % shards.size() ]->Delete(key);
}

// Implements Afina::Storage interface
bool StripedLRU::Get(const std::string &key, std::string &value)  {
    return shards[hash_func(key) % shards.size() ]->Get(key, value);
}

} //namespace Backend
} //namespace Afina
