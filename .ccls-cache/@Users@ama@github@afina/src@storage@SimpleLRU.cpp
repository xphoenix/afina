#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

void
SimpleLRU::_free_data(size_t size)
{
    while (size > _free_size) {
        _free_size += _lru_head->key.size() + _lru_head->value.size();
        _lru_index.errase(_lru_head->key);
        _lru_head.swap(_lru_head->next);
        _lru_head->prev = nullptr;
        _lru_head->next.reset();
    }
}

void
SimpleLRU::_move_to_tail(lru_node &elem)
{
    if (elem.next == nullptr) {
        return ;
    }
    if (elem.prev == nullptr) {
        elem.next->prev = nullptr;
        elem.next.swap(_lru_head);
        _lru_tail->next.swap(elem.next);
        elem.prev = _lru_tail;
        _lru_tail = &elem;
        return ;
    }

    elem.next.swap(elem.prev->next);
    elem.prev->next->prev = elem.prev;
    elem.next.swap(_lru_tail->next);
    elem.prev = _lru_tail;
    _lru_tail = &elem;
    return ;
}

bool
SimpleLRU::_Set(const std::string &key, const std::string &value, lru_index_iterator &now)
{
    if ((key.size() + value.size()) > _max_size) {
        return false;
    }
    _move_to_tail(now->second.get());

    if (value.size() - _lru_tail->value.size() > 0) {
        _free_data(value.size() - _lru_tail->value.size());
    }
    _free_size += _lru_tail->value.size() - value.size();

    _lru_tail->value = value;
    return true;
}

bool
SimpleLRU::_PutIfAbsent(const std::string &key, const std::string &value)
{
    if ((key.size() + value.size()) > _max_size) {
        return false;
    }
    _free_data(key.size() + value.size());

    _free_size -= key.size() + value.size();

    lru_node *obj = new lru_node{key, value, nullptr, nullptr};
    _lru_index.insert(std::reference_wrapper<const std::string>(obj->key), std::reference_wrapper<lru_node>(*obj)});

    if (_lru_tail == nullptr) {
        _lru_tail = obj;
        _lru_head.reset(obj);
    } else {
        _lru_tail->next.reset(obj);
        obj->prev = _lru_tail;
        _lru_tail = obj;
    }
    return true;
}

// See MapBasedGlobalLockImpl.h
bool
SimpleLRU::Put(const std::string &key, const std::string &value)
{
    lru_index_iterator find_iter = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (find_iter == _lru_index.end()) {
        return _PutIfAbsent(key, value);
    } else {
        return _Set(key, value, find_iter);
    }
}

// See MapBasedGlobalLockImpl.h
bool
SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    lru_index_iterator find_iter = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (find_iter == _lru_index.end()) {
        return _PutIfAbsent(key, value);
    } else {
        return false;
    }
}

// See MapBasedGlobalLockImpl.h
bool
SimpleLRU::Set(const std::string &key, const std::string &value)
{
    lru_index_iterator find_iter = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (find_iter == _lru_index.end()) {
        return false;
    } else {
        return _Set(key, value, find_iter);
    }
}

// See MapBasedGlobalLockImpl.h
bool
SimpleLRU::Delete(const std::string &key)
{
    lru_index_iterator find_iter = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (find_iter == _lru_index.end()) {
        return false;
    }

    lru_node &now = find_iter->second.get();
    _free_size += (now.value.size() + now.key.size());
    _lru_index.erase(key);

    if (now.prev == nullptr) {
        now.next.swap(_lru_head);
        _lru_head->prev = nullptr;
        now.next.reset();
        delete(now);
        return true;
    }

    if (now.next == nullptr) {
        _lru_tail = now.prev;
        now.prev->next.swap(now.next);
        now.next.reset();
        delete(now);
        return true;
    }

    now.prev->next.swap(now.next);
    now.prev->next->prev = now.prev;
    now.next.reset();
    delete(now);
    return 1;
}

// See MapBasedGlobalLockImpl.h
bool
SimpleLRU::Get(const std::string &key, std::string &value)
{
    lru_index_iterator find_iter = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (find_iter == _lru_index.end()) {
        return false;
    } else {
        _move_to_tail(find_iter->second.get());
        value = find_iter->second.get().value;
        return true;
    }
}

} // namespace Backend
} // namespace Afina
