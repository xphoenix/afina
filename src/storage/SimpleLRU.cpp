#include "SimpleLRU.h"


namespace Afina {
    namespace Backend {

// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Put(const std::string &key, const std::string &value) {

            auto it = _lru_index.find(key);
            bool isExist = (it != _lru_index.end());

            if (isExist)
                return setAndReorder(key, value);
            else
                return addNode(key, value);
        }


// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {

            auto it = _lru_index.find(key);
            bool isExist = (it != _lru_index.end());

            if (isExist)
                return false;
            else
                return addNode(key, value);
        }


// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Set(const std::string &key, const std::string &value) {

            auto it = _lru_index.find(key);
            bool isExist = (it != _lru_index.end());

            if (isExist) {
                return setAndReorder(key, value);
            }
            return isExist;
        }


// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Delete(const std::string &key) {

            auto it = _lru_index.find(key);
            bool isExist = (it != _lru_index.end());

            if (isExist) {
                auto node = it->second;
                actual_cache_size -= (node.get().key.size() + node.get().value.size());

                if (&node.get() == _lru_head.get()) {
                    std::unique_ptr<lru_node> tmp(std::move(_lru_head));
                    _lru_head = std::move(tmp->next);
                } else if (&node.get() == _lru_tail){
                    _lru_tail = _lru_tail->prev;
                    _lru_tail->next.reset();
                }
                else {
                    node.get().next->prev = node.get().prev;
                    node.get().prev->next.swap(node.get().next);
                    node.get().next.reset();
                }

                _lru_index.erase(it);
            }
            return isExist;
        }


// See MapBasedGlobalLockImpl.h
        bool SimpleLRU::Get(const std::string &key, std::string &value) {

            auto it = _lru_index.find(key);
            bool isExist = (it != _lru_index.end());

            if (isExist) {
                value = it->second.get().value;
                reorderCache(it->second);
            }
            return isExist;
        }


        void SimpleLRU::reorderCache(std::reference_wrapper<lru_node> lastRefNode) {

            lru_node& lrnode = lastRefNode.get();
            if (_lru_tail != &lrnode) {

                if (_lru_head.get() != &lrnode) {
                    lrnode.next->prev = lrnode.prev;
                    lrnode.prev->next.swap(lrnode.next);
                } else {
                    _lru_head.swap(lrnode.next);
                    _lru_head->prev = nullptr;
                }

                _lru_tail->next.swap(lrnode.next);
                lrnode.prev = _lru_tail;
                _lru_tail = &lrnode;
            }
        }


        bool SimpleLRU::setAndReorder(const std::string &key, const std::string &value) {
            auto it = _lru_index.find(key);
            auto node = it->second;
            if (node.get().key.size() + value.size() > _max_size)
                return false;

            while (node.get().key.size() + value.size() - node.get().value.size() + actual_cache_size > _max_size)
                removeHeadNode();

            node.get().value = value;

            reorderCache(node);
            return true;
        }


        void SimpleLRU::removeHeadNode() {
            actual_cache_size -= _lru_head->key.size() + _lru_head->value.size();
            _lru_index.erase(_lru_head->key);

            if (_lru_head->next != nullptr) {
                _lru_head->next->prev = _lru_head->prev;
            }

            // leak?
            std::unique_ptr<lru_node> headToDelete(std::move(_lru_head));
            _lru_head = std::move(headToDelete->next);
        }


        bool SimpleLRU::addNode(const std::string &key, const std::string &value) {
            if ((key.size() + value.size()) > _max_size) {
                return false;
            }

            while (key.size() + value.size() + actual_cache_size > _max_size) {
                removeHeadNode();
            }

            std::unique_ptr<lru_node> new_node (new lru_node{key, value, nullptr, nullptr});
            if (_lru_tail != nullptr) {
                new_node->prev = _lru_tail;
                _lru_tail->next.swap(new_node);
                _lru_tail = _lru_tail->next.get();
            } else {
                _lru_head.swap(new_node);
                _lru_tail = _lru_head.get();
            }

            _lru_index.insert(std::make_pair(std::ref(_lru_tail->key), std::ref(*_lru_tail)));
            actual_cache_size += key.size() + value.size();
            return true;
        }

    } // namespace Backend
} // namespace Afina
