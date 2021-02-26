#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

  // See MapBasedGlobalLockImpl.h
  bool SimpleLRU::_overflow(size_t new_size) const
  {
      return new_size > _max_size;
  }

  // See MapBasedGlobalLockImpl.h
  void SimpleLRU::_insert_node(std::unique_ptr<lru_node> &node)
  {
      if (_lru_head->next == nullptr) {
          _lru_head->next = std::move(node);
          _lru_head->next->prev = _lru_head.get();
          _lru_tail = _lru_head->next.get();
      } else {
          _lru_head->next->prev = node.get();
          node->next = std::move(_lru_head->next);
          node->prev = _lru_head.get();
          _lru_head->next = std::move(node);
      }
  }

  // See MapBasedGlobalLockImpl.h
  void SimpleLRU::_get_up(lru_node *cur)
  {
      // if current node is already in head
      if (cur == _lru_head->next.get()) {
          return;
      }

      // updated tail node
      if (cur == _lru_tail) {
          _lru_tail = cur->prev;
      }

      // head - cur <- node
      _lru_head->next->prev = cur;
      if (cur->next != nullptr) {
          cur->next->prev = cur->prev;
      }

      // head -> cur <- node
      lru_node *second = _lru_head->next.release();
      _lru_head->next.reset(cur);

      // update next of prev_of_current
      cur->prev->next.release();
      lru_node *center_next = cur->next.release();
      cur->prev->next.reset(center_next);

      // insert current after head
      cur->prev = _lru_head.get();
      cur->next.release();
      cur->next.reset(second);
  }


  bool SimpleLRU::_put_node(const std::string &key, const std::string &value)
  {
      size_t node_size = key.size() + value.size();

      // too large node
      if (_overflow(node_size)) {
          return false;
      }

      // if we need more space for new node, delete old nodes while too few space
      if (_overflow(_cur_size + node_size)) {
          while (_overflow(_cur_size + node_size)) {
              _lru_index.erase(std::cref(_lru_tail->key));
              _cur_size  = _cur_size - _lru_tail->key.size() - _lru_tail->value.size();
              _lru_tail = _lru_tail->prev;
              _lru_tail->next.reset(nullptr);
          }
      }

      auto node = std::unique_ptr<lru_node>( new lru_node(key, value) );

      auto pair = _lru_index.emplace(std::make_pair(std::cref(node->key), std::ref(*node)));
      bool res = pair.second;
      if (!res) {
          return false;
      }

      _cur_size += key.size() + value.size();

      // insert node in the top of the list
      _insert_node(node);
      return true;
  }

  bool SimpleLRU::_set_node(const std::string &key, const std::string &value, index::iterator &it_find)
  {
      auto node_ref = it_find->second;
      size_t node_value_size = node_ref.get().value.size();
      size_t node_size = key.size() + value.size();

      // if too large node
      if (_overflow(node_size)) {
          return false;
      }

      node_ref.get().value = value;

      lru_node *cur = &node_ref.get();
      _get_up(cur);

      _cur_size = _cur_size - node_value_size + value.size();

      // delete old nodes while too few space.
      while (_overflow(_cur_size)) {
          _lru_index.erase(std::cref(_lru_tail->key));
          _cur_size  = _cur_size - _lru_tail->key.size() - _lru_tail->value.size();
          _lru_tail = _lru_tail->prev;
          _lru_tail->next.reset(nullptr);
      }
      return true;
  }


  bool SimpleLRU::Put(const std::string &key, const std::string &value) {
      auto it_find = _lru_index.find(std::cref(key));
      if (it_find == _lru_index.end()) {
          return _put_node(key, value);
      } else {
          return _set_node(key, value, it_find);
      }
  }


  // See MapBasedGlobalLockImpl.h
  bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
      auto it_find = _lru_index.find(std::cref(key));
      if (it_find == _lru_index.end()) {
          return _put_node(key, value);
      }
      return false;
  }

  // See MapBasedGlobalLockImpl.h
  bool SimpleLRU::Set(const std::string &key, const std::string &value) {
      auto it_find = _lru_index.find(std::cref(key));
      if (it_find != _lru_index.end()) {
          return _set_node(key, value, it_find);
      }
      return false;
  }

  // See MapBasedGlobalLockImpl.h
  bool SimpleLRU::Delete(const std::string &key)
  {
      auto it_find = _lru_index.find(std::cref(key));
      if (it_find == _lru_index.end()) {
          return false;
      }

      lru_node *cur = &it_find->second.get();
      _cur_size = _cur_size - cur->value.size() - cur->key.size();

      if (cur->next != nullptr) {
          cur->next->prev = cur->prev;
          cur->prev->next = std::move(cur->next);
      } else {
          _lru_tail = cur->prev;
          cur->prev->next = nullptr;
      }

      _lru_index.erase(it_find);

      return true;
  }

  // See MapBasedGlobalLockImpl.h
  bool SimpleLRU::Get(const std::string &key, std::string &value)
  {
      auto it_find = _lru_index.find(std::cref(key));
      if (it_find == _lru_index.end()) {
          return false;
      } else {
          lru_node *cur = &it_find->second.get();
          _get_up(cur);
          value = cur->value;
          return true;
      }
  }

} // namespace Backend
} // namespace Afina
