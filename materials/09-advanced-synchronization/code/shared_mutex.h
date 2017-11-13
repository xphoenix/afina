#ifndef SHARED_MUTEX_H
#define SHARED_MUTEX_H

#include <chrono>
#include <climits>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

// C++11 version of std::exchange for internal use.
template <typename _Tp, typename _Up = _Tp> inline _Tp __exchange(_Tp &__obj, _Up &&__new_val) {
    _Tp __old_val = std::move(__obj);
    __obj = std::forward<_Up>(__new_val);
    return __old_val;
}

/// Assign @p __new_val to @p __obj and return its previous value.
template <typename _Tp, typename _Up = _Tp> inline _Tp exchange(_Tp &__obj, _Up &&__new_val) {
    return std::__exchange(__obj, std::forward<_Up>(__new_val));
}

// A movable (multi-scope) RAII wrapper for share-locking and share-unlocking mutexes supporting
//   shared ownership such as shared_mutex and upgrade_mutex.  unique_lock is used for locking and unlocking
//   such mutexes for unique (write) ownership.
template <typename _Mutex> class shared_lock {
public:
    typedef _Mutex mutex_type;

    // Shared locking

    shared_lock() noexcept : _M_pm(nullptr), _M_owns(false) {}

    explicit shared_lock(mutex_type &__m) : _M_pm(std::__addressof(__m)), _M_owns(true) { __m.lock_shared(); }

    shared_lock(mutex_type &__m, std::defer_lock_t) noexcept : _M_pm(std::__addressof(__m)), _M_owns(false) {}

    shared_lock(mutex_type &__m, std::try_to_lock_t) : _M_pm(std::__addressof(__m)), _M_owns(__m.try_lock_shared()) {}

    shared_lock(mutex_type &__m, std::adopt_lock_t) : _M_pm(std::__addressof(__m)), _M_owns(true) {}

    template <typename _Clock, typename _Duration>
    shared_lock(mutex_type &__m, const std::chrono::time_point<_Clock, _Duration> &__abs_time)
        : _M_pm(std::__addressof(__m)), _M_owns(__m.try_lock_shared_until(__abs_time)) {}

    template <typename _Rep, typename _Period>
    shared_lock(mutex_type &__m, const std::chrono::duration<_Rep, _Period> &__rel_time)
        : _M_pm(std::__addressof(__m)), _M_owns(__m.try_lock_shared_for(__rel_time)) {}

    ~shared_lock() {
        if (_M_owns)
            _M_pm->unlock_shared();
    }

    shared_lock(shared_lock const &) = delete;
    shared_lock &operator=(shared_lock const &) = delete;

    shared_lock(shared_lock &&__sl) noexcept : shared_lock() { swap(__sl); }

    shared_lock &operator=(shared_lock &&__sl) noexcept {
        shared_lock(std::move(__sl)).swap(*this);
        return *this;
    }

    void lock() {
        _M_lockable();
        _M_pm->lock_shared();
        _M_owns = true;
    }

    bool try_lock() {
        _M_lockable();
        return _M_owns = _M_pm->try_lock_shared();
    }

    template <typename _Rep, typename _Period>
    bool try_lock_for(const std::chrono::duration<_Rep, _Period> &__rel_time) {
        _M_lockable();
        return _M_owns = _M_pm->try_lock_shared_for(__rel_time);
    }

    template <typename _Clock, typename _Duration>
    bool try_lock_until(const std::chrono::time_point<_Clock, _Duration> &__abs_time) {
        _M_lockable();
        return _M_owns = _M_pm->try_lock_shared_until(__abs_time);
    }

    void unlock() {
        if (!_M_owns)
            throw std::runtime_error("Unlock not owned lock");
        _M_pm->unlock_shared();
        _M_owns = false;
    }

    // Setters

    void swap(shared_lock &__u) noexcept {
        std::swap(_M_pm, __u._M_pm);
        std::swap(_M_owns, __u._M_owns);
    }

    mutex_type *release() noexcept {
        _M_owns = false;
        return exchange(_M_pm, nullptr);
    }

    // Getters

    bool owns_lock() const noexcept { return _M_owns; }

    explicit operator bool() const noexcept { return _M_owns; }

    mutex_type *mutex() const noexcept { return _M_pm; }

private:
    void _M_lockable() const {
        if (_M_pm == nullptr)
            throw std::runtime_error("No lock");
        if (_M_owns)
            throw std::runtime_error("deadlock");
    }

    mutex_type *_M_pm;
    bool _M_owns;
};

/// Swap specialization for shared_lock
template <typename _Mutex> void swap(shared_lock<_Mutex> &__x, shared_lock<_Mutex> &__y) noexcept { __x.swap(__y); }

// A mutex supporting both unique (write) and shared (read) ownership
class shared_mutex {
public:
    shared_mutex() : state_(0) {}

    // Exclusive ownership
    void lock();
    bool try_lock();
    bool timed_lock(std::chrono::nanoseconds rel_time);
    void unlock();

    // Shared ownership
    void lock_shared();
    bool try_lock_shared();
    bool timed_lock_shared(std::chrono::nanoseconds rel_time);
    void unlock_shared();

private:
    std::mutex mut_;
    std::condition_variable gate1_;
    std::condition_variable gate2_;
    unsigned state_;

    static constexpr unsigned write_entered_ = 1U << (sizeof(unsigned) * CHAR_BIT - 1);
    static constexpr unsigned n_readers_ = ~write_entered_;
};

#endif // SHARED_MUTEX_H
