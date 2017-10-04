#include "MapBasedGlobalLockImpl.h"

#include <mutex>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
    std::unique_lock<std::mutex> guard(_lock);
	
	if (_backend.find(key) != _backend.end()) {
		_backend.erase(key);
		_backend[key] = value;
		_order.remove(key);
		_order.push_front(key);
	}
	else if ( _max_size == _order.size() ) {
		std::string buf = _order.back();
		_order.pop_back();
		_order.push_front(key);
		_backend[key] = value;
		}
		else {
		_order.push_front(key);
		_backend[key] = value;
		}

    return true;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
    std::unique_lock<std::mutex> guard(_lock);

	if ( _backend.find(key) == _backend.end() ) {
		if ( _max_size == _order.size() ) {
		std::string buf = _order.back();
		_order.pop_back();
		_order.push_front(key);
		_backend[key] = value;
		}
		else {
		_order.push_front(key);
		_backend[key] = value;
		}

		return true;
	}

    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
    std::unique_lock<std::mutex> guard(_lock);
	if ( _backend.find(key) != _backend.end() ) {
		_order.push_front(key);
		_backend[key] = value;

		return true;
	}

    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) {
    std::unique_lock<std::mutex> guard(_lock);

	if ( _backend.find(key) != _backend.end() ) {
		_order.remove(key);
		_backend.erase(key);

		return true;
	}

    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
    std::unique_lock<std::mutex> guard(*const_cast<std::mutex *>(&_lock));

	if ( _backend.find(key) != _backend.end() ) {
		value = _backend.at(key);

		return true;
	}

    return false;
}

} // namespace Backend
} // namespace Afina
