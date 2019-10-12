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
        _readers(0),
        _writers(0)
        {}
    ~ThreadSafeSimplLRU() {}

    void lock(){
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
    }

    void unlock(){
        {
            std::unique_lock<std::mutex> locker(_mutex);
              _writers--;
        }
            _cv_read.notify_all();
    }

    void shared_lock(){
        std::unique_lock<std::mutex> locker(_mutex);
        while (_writers != 0)
        {
            _cv_read.wait(locker);
        }
        _readers++;
    }

    void shared_unlock(){
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

    // see SimpleLRU.h
    bool Put(const std::string &key, const std::string &value) override {
        lock();
        bool result;
        {
            std::unique_lock<std::mutex> locker(_mutex);
            result = SimpleLRU::Put(key, value);
        }
        unlock();
        return result;
    }

    // see SimpleLRU.h
    bool PutIfAbsent(const std::string &key, const std::string &value) override {
        lock();
        bool result;
        {
            std::unique_lock<std::mutex> locker(_mutex);
            result = SimpleLRU::PutIfAbsent(key, value);
        }
        unlock();
        return result;
    }

    // see SimpleLRU.h
    bool Set(const std::string &key, const std::string &value) override {
        lock();
        bool result;
        {
            std::unique_lock<std::mutex> locker(_mutex);
            result = SimpleLRU::Set(key, value);
        }
        unlock();
        return result;
    }

    // see SimpleLRU.h
    bool Delete(const std::string &key) override {
        lock();
        bool result;
        {
            std::unique_lock<std::mutex> locker(_mutex);
            result = SimpleLRU::Delete(key);
        }
        unlock();
        return result;
    }

    // see SimpleLRU.h
    bool Get(const std::string &key, std::string &value) override {
        shared_lock();
        bool result = SimpleLRU::Get(key, value);
        shared_unlock();
        return result;
    }

private:
    // Mutex for storage operations
    std::mutex _mutex;

    // Number of threads that wait to get data
    uint32_t _readers;

    // Number of threads that wait to put/set/delete data
    uint32_t _writers;

    //
    std::condition_variable _cv_write;

    std::condition_variable _cv_read;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H
