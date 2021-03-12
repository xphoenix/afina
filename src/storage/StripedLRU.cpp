//
// Created by Дмитрий Калашников on 11.03.2021.
//

#include "StripedLRU.h"

namespace Afina {
namespace Backend {

StripedLRU::StripedLRU(std::size_t memory_limit, std::size_t stripe_count)
{
    size_t stripe_size = memory_limit / stripe_count;
    for (int i = 0; i < stripe_count; i++) {
        _shards.emplace_back(stripe_size);
    }
}

// Implements Afina::Storage interface
bool StripedLRU::Put(const std::string &key, const std::string &value)
{
    return _shards[hash(key) % _shards.size()].Put(key, value);
}

// Implements Afina::Storage interface
bool StripedLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    return _shards[hash(key) % _shards.size()].PutIfAbsent(key, value);
}

// Implements Afina::Storage interface
bool StripedLRU::Set(const std::string &key, const std::string &value)
{
    return _shards[hash(key) % _shards.size()].Set(key, value);
}

// Implements Afina::Storage interface
bool StripedLRU::Delete(const std::string &key)
{
    return _shards[hash(key) % _shards.size()].Delete(key);
}

// Implements Afina::Storage interface
bool StripedLRU::Get(const std::string &key, std::string &value)
{
    return _shards[hash(key) % _shards.size()].Get(key, value);
}

} // namespace Backend
} // namespace Afina