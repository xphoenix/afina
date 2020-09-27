#include "SimpleLRU.h"

namespace Afina {
  namespace Backend {

    // See MapBasedGlobalLockImpl.h
    bool SimpleLRU::Put(const std::string & key,
      const std::string & value) {

      if (key.size() + value.size() > _max_size) {
        return false;
      }

      if (!_lru_head) {
        _lru_head.reset(new lru_node(key, value));
        _lru_head -> prev = _lru_head.get();
        _lru_index.emplace(_lru_head -> prev -> key, * _lru_head -> prev);
        _current_size = key.size() + value.size();
      } else {
        if (_lru_index.find(key) == _lru_index.end()) {
          while (_current_size + key.size() + value.size() > _max_size) {
            lru_node * new_head = _lru_head -> next.get();
            new_head -> prev = _lru_head -> prev;
            _lru_head -> next.release();
            _lru_index.erase(_lru_head -> key);
            _lru_head.reset(new_head);
            _current_size -= (_lru_head -> key.size() + _lru_head -> value.size());

          }
          lru_node * last = _lru_head -> prev;
          last -> next.reset(new lru_node(key, value));
          last -> next -> prev = last;
          _lru_head -> prev = last -> next.get();
          _lru_index.emplace(_lru_head -> prev -> key, *(last -> next));
          _current_size = _current_size + key.size() + value.size();
        } else {

          while (_current_size + value.size() - _lru_index.find(key) -> second.get().value.size() > _max_size) {
            lru_node * new_head = _lru_head -> next.get();
            new_head -> prev = _lru_head -> prev;
            _lru_head -> next.release();
            _lru_index.erase(_lru_head -> key);
            _lru_head.reset(new_head);
            _current_size -= (_lru_head -> key.size() + _lru_head -> value.size());

          }
          auto & prev_value = _lru_index.find(key) -> second.get().value;
          prev_value = value;
          _current_size = _current_size + value.size() - _lru_index.find(key) -> second.get().value.size();

        }
      }

      return true;

    }

    // See MapBasedGlobalLockImpl.h
    bool SimpleLRU::PutIfAbsent(const std::string & key,
      const std::string & value) {
      if (key.size() + value.size() > _max_size) {
        return false;
      }

      if (!_lru_head) {
        _lru_head.reset(new lru_node(key, value));
        _lru_head -> prev = _lru_head.get();
        _lru_index.emplace(_lru_head -> prev -> key, * _lru_head -> prev);
        _current_size = (key + value).size();
      } else {
        if (_lru_index.find(key) != _lru_index.end()) {
          return false;
        } else {

          while (_current_size + (key + value).size() > _max_size) {
            lru_node * new_head = _lru_head -> next.get();
            new_head -> prev = _lru_head -> prev;
            _lru_head -> next.release();
            _lru_index.erase(_lru_head -> key);
            _lru_head.reset(new_head);
            _current_size -= (_lru_head -> key.size() + _lru_head -> value.size());

          }

          lru_node * last = _lru_head -> prev;
          last -> next.reset(new lru_node(key, value));
          last -> next -> prev = last;
          _lru_head -> prev = last -> next.get();
          _lru_index.emplace(_lru_head -> prev -> key, *(last -> next));
          _current_size = _current_size + key.size() + value.size();
        }

      }

      return true;
    }

    // See MapBasedGlobalLockImpl.h
    bool SimpleLRU::Set(const std::string & key,
      const std::string & value) {
      if (key.size() + value.size() > _max_size) {
        return false;
      }

      if (_lru_index.find(key) == _lru_index.end()) {
        return false;
      } else {
        while (_current_size + value.size() - _lru_index.find(key) -> second.get().value.size() > _max_size) {
          lru_node * new_head = _lru_head -> next.get();
          _lru_head -> next.release();
          new_head -> prev = _lru_head -> prev;
          _lru_index.erase(_lru_head -> key);
          _lru_head.reset(new_head);
          _current_size -= (_lru_head -> key.size() + _lru_head -> value.size());

        }
        auto & prev_value = _lru_index.find(key) -> second.get().value;
        prev_value = value;
        _current_size = _current_size + value.size() - _lru_index.find(key) -> second.get().value.size();

      }

      return true;
    }

    // See MapBasedGlobalLockImpl.h
    bool SimpleLRU::Delete(const std::string & key) {

      if (_lru_index.find(key) == _lru_index.end()) {
        return false;
      }

      auto & lru_node = _lru_index.find(key) -> second.get();
      auto next_node = lru_node.next.get();
      auto prev_node = lru_node.prev;
      _lru_index.erase(key);
      _current_size -= key.size() + lru_node.value.size();

      if (next_node) {

        next_node -> prev = prev_node;

        if (key == _lru_head -> key) {

          _lru_head -> next.release();
          _lru_head.reset(next_node);

        } else {
          prev_node -> next.reset(next_node);
        }
      } else {

        _lru_head -> prev = prev_node;
        if (key == _lru_head -> key) {
          _lru_head -> next.release();
          _lru_head.reset(next_node);
        } else {
          prev_node -> next.reset(next_node);
        }
      }

      return true;
    }

    // See MapBasedGlobalLockImpl.h
    bool SimpleLRU::Get(const std::string & key, std::string & value) {
      if (_lru_index.find(key) == _lru_index.end()) {
        return false;
      }
      value = _lru_index.find(key) -> second.get().value;
      return true;
    }

  } // namespace Backend
} // namespace Afina
