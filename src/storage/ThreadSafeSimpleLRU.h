#ifndef AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H
#define AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H

#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>
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
    ThreadSafeSimplLRU(size_t max_size = 1024) :
        SimpleLRU(max_size),
        _read_flag(false),
        _readers(0),
        _writers(0)
        {}
    ~ThreadSafeSimplLRU() {}

    // see SimpleLRU.h
    bool Put(const std::string &key, const std::string &value) override {
        bool result;
        {
            std::unique_lock<std::mutex> locker(_mutex);
            while(_writers != 0)
            {
                _cv_read.wait(locker);
            }
            _writers++;
            while(_readers != 0)
            {
                _cv_write.wait(locker);
            }
            result = SimpleLRU::Put(key, value);
            _writers--;
        }
        _cv_read.notify_all();
        return result;
    }

    // see SimpleLRU.h
    bool PutIfAbsent(const std::string &key, const std::string &value) override {
        bool result;
        {
            std::unique_lock<std::mutex> locker(_mutex);
            while(_writers != 0)
            {
                _cv_read.wait(locker);
            }
            _writers++;
            while(_readers != 0)
            {
                _cv_write.wait(locker);
            }
            result = SimpleLRU::PutIfAbsent(key, value);
            _writers--;
        }
        _cv_read.notify_all();
        return result;
    }

    // see SimpleLRU.h
    bool Set(const std::string &key, const std::string &value) override {
        bool result;
        {
            std::unique_lock<std::mutex> locker(_mutex);
            while(_writers != 0)
            {
                _cv_read.wait(locker);
            }
            _writers++;
            while(_readers != 0)
            {
                _cv_write.wait(locker);
            }
            result = SimpleLRU::Set(key, value);
            _writers--;
        }
        _cv_read.notify_all();
        return result;
    }

    // see SimpleLRU.h
    bool Delete(const std::string &key) override {
        bool result;
        {
            std::unique_lock<std::mutex> locker(_mutex);
            while(_writers != 0)
            {
                _cv_read.wait(locker);
            }
            _writers++;
            while(_readers != 0)
            {
                _cv_write.wait(locker);
            }
            result = SimpleLRU::Delete(key);
            _writers--;
        }
        _cv_read.notify_all();
        return result;
    }

    // see SimpleLRU.h
    bool Get(const std::string &key, std::string &value) override {
        {
            std::unique_lock<std::mutex> locker(_mutex);
            while (_writers != 0)
            {
                _cv_read.wait(locker);
            }
            _readers++;

        }
        auto result = SimpleLRU::Get(key, value);
        {
            std::unique_lock<std::mutex> locker(_mutex);
            _readers--;
            if (_writers != 0)
            {
                if (_readers == 0)
                {
                    _cv_write.notify_one();
                }
            }
        }
        return result;
    }

private:
    // Mutex for storage operations
    std::mutex _mutex;

    std::atomic<bool> _read_flag;

    uint32_t _readers;

    uint32_t _writers;

    //
    std::condition_variable _cv_write;

    std::condition_variable _cv_read;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H
