//
// Created by Дмитрий Калашников on 11.03.2021.
//

#ifndef AFINA_STRIPEDLRU_H
#define AFINA_STRIPEDLRU_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <functional>

#include <afina/Storage.h>

#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

class StripedLRU : public Afina::Storage {
public:

    static StripedLRU create_cache(std::size_t stripe_count, std::size_t memory_limit)
    {
        constexpr size_t min_memory_size = 1u * 1024 * 1024;
        if (memory_limit / stripe_count < min_memory_size || memory_limit % stripe_count != 0) {
            throw std::runtime_error("Invalid memory limit");
        }
        return StripedLRU(stripe_count, memory_limit);
    }

    ~StripedLRU() {}

    // Implements Afina::Storage interface
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) override;

private:

    std::vector<SimpleLRU> _shards;
    std::hash<std::string> hash;

    StripedLRU(std::size_t memory_limit, std::size_t stripe_count);

};

} // namespace Backend
} // namespace Afina


#endif // AFINA_STRIPEDLRU_H
