#ifndef AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H
#define AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H

#include <map>
#include <mutex>
#include <string>

#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

/**
 * # SimpleLRU thread safe version
 *
 *
 */
class ThreadSafeSimplLRU : public SimpleLRU {
public:
    ThreadSafeSimplLRU(size_t max_size = 1024) : SimpleLRU(max_size) {}
    ~ThreadSafeSimplLRU() {}

    // see SimpleLRU.h
    bool Put(const std::string &key, const std::string &value) override {
        // TODO: sinchronization
        std::lock_guard<std::mutex> lock(_my_mutex); //here _my_mutex is locked (not oly introduced)
        return SimpleLRU::Put(key, value);
    }

    // see SimpleLRU.h
    bool PutIfAbsent(const std::string &key, const std::string &value) override {
        // TODO: sinchronization
        std::lock_guard<std::mutex> lock(_my_mutex);
        return SimpleLRU::PutIfAbsent(key, value);
    }

    // see SimpleLRU.h
    bool Set(const std::string &key, const std::string &value) override {
        // TODO: sinchronization
        std::lock_guard<std::mutex> lock(_my_mutex);
        return SimpleLRU::Set(key, value);
    }

    // see SimpleLRU.h
    bool Delete(const std::string &key) override {
        // TODO: sinchronization
        std::lock_guard<std::mutex> lock(_my_mutex);
        return SimpleLRU::Delete(key);
    }

    // see SimpleLRU.h
    bool Get(const std::string &key, std::string &value) override {
        // TODO: sinchronization
        std::lock_guard<std::mutex> lock(_my_mutex);
        return SimpleLRU::Get(key, value);
    }

private:
    // TODO: sinchronization primitives
    std::mutex _my_mutex;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H
