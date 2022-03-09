#include "StripedLRU.h"

namespace Afina {
namespace Backend {

StripedLRU::StripedLRU(size_t max_size, size_t strip_count) : strip_count(strip_count) {
    size_t strip_size = max_size / strip_count;
    // TODO: choose size
    if (strip_size < 2) {
        throw std::runtime_error("Invalid parameters: <max_size>, <strip_count>");
    }
    shards = std::vector<ThreadSafeSimplLRU>(strip_count, ThreadSafeSimplLRU(strip_size));
}

StripedLRU::~StripedLRU() {}

bool StripedLRU::Put(const std::string &key, const std::string &value) {
    size_t shard_index = hasher(key) % strip_count;
    return shards[shard_index].Put(key, value);
}

bool StripedLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    size_t shard_index = hasher(key) % strip_count;
    return shards[shard_index].PutIfAbsent(key, value);
}

bool StripedLRU::Set(const std::string &key, const std::string &value) {
    size_t shard_index = hasher(key) % strip_count;
    return shards[shard_index].Set(key, value);
}

bool StripedLRU::Delete(const std::string &key) {
    size_t shard_index = hasher(key) % strip_count;
    return shards[shard_index].Delete(key);
}
bool StripedLRU::Get(const std::string &key, std::string &value) {
    size_t shard_index = hasher(key) % strip_count;
    return shards[shard_index].Get(key, value);
}
} // namespace Backend
} // namespace Afina