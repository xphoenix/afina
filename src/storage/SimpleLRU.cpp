#include "SimpleLRU.h"

namespace Afina
{
namespace Backend
{

    /*
    We need to write 3 secondary fuctions:
    1) delete last element ftom the list;
    2) insert element at the beginnig (key doesn't exist);
    3) key exists => we need to transfer element at the beginning.
    It's very simple.
    */

    void SimpleLRU::delete_last()
    {
        lru_node* del_node = _lru_tail;
        std::size_t delta_sz = del_node->key.size()
            + del_node->value.size(); // size of cleaned memory after deleting
        _lru_index.erase(del_node->key); // delete from the map, first of all (it's link)
        if (_lru_head.get() == _lru_tail) // one element in the list
        {
            _lru_head.reset(nullptr);
        }
        else
        {
            _lru_tail = _lru_tail->prev; // reset tail
            _lru_tail->next.reset(nullptr);
        }
        _cur_size -= delta_sz; // recalculate memory
    }

    void SimpleLRU::insert_node(lru_node* node)
    {
        if (_lru_head.get())
        {
            _lru_head.get()->prev = node;
        }
        else // The list is empty
        {
            _lru_tail = node;
        }
        node->next = std::move(_lru_head); // we can't copy unique_ptr
        _lru_head.reset(node); // make new head
    }

    void SimpleLRU::node_to_head(lru_node* node)
    {
        if (_lru_head.get() == node) // Node is already head
        {
            return;
        }
        if (!node->next.get()) // one element in the list
        {
            _lru_tail = node->prev;
            _lru_head.get()->prev = node;
            node->next = std::move(_lru_head);
            _lru_head = std::move(node->prev->next);
        }
        else
        {
            auto tmp = std::move(_lru_head);
            _lru_head = std::move(node->prev->next);
            node->prev->next = std::move(node->next);
            tmp.get()->prev = node;
            node->next = std::move(tmp);
            node->prev->next->prev = node->prev;
        }
    }


    bool SimpleLRU::Put(const std::string& key, const std::string& value)
    {
        std::size_t block_size = key.size() + value.size(); // memory for new block
        if (block_size > _max_size) // very big block
        {
            return false;
        }
        auto node = _lru_index.find(key); // check key in the list
        if (node != _lru_index.end()) // key exists
        {
            int delta_sz = value.size() - (node->second).get().value.size(); // second - node's link
            node_to_head(&node->second.get()); // very usefull function...
            while (_cur_size + delta_sz > _max_size) // free memory for new block
            {
                delete_last();
            }
            node->second.get().value = value;
            _cur_size += delta_sz; // recalculate memory
        }
        else // key doesn't exist, make new block
        {
            while (_cur_size + block_size > _max_size)
            {
                delete_last();
            }
            lru_node* new_node = new lru_node{ key, value, nullptr, nullptr };
            insert_node(new_node); // TODO reset in unique_ptr
            _lru_index.insert({ std::reference_wrapper<const std::string>(new_node->key),
                std::reference_wrapper<lru_node>(
                                    *new_node) }); // insert in the map for search optimization
            _cur_size += block_size; // recalculate memory
        }
        return true; // It's OK.
    }

    bool SimpleLRU::PutIfAbsent(const std::string& key, const std::string& value)
    {
        if (_lru_index.find(key) == _lru_index.end())
        {
            return Put(key, value);
        }
        else
        {
            return false;
        }
    }

    bool SimpleLRU::Set(const std::string& key, const std::string& value)
    {
        if (_lru_index.find(key) != _lru_index.end())
        {
            return Put(key, value);
        }
        else
        {
            return false;
        }
    }

    // See MapBasedGlobalLockImpl.h
    bool SimpleLRU::Delete(const std::string& key)
    {
        auto tmp = _lru_index.find(key);
        if (tmp == _lru_index.end()) // key check
        {
            return false;
        }
        lru_node* node = &(tmp->second.get());
        _lru_index.erase(node->key); // delete from the map
        std::size_t node_size = node->key.size() + node->value.size();
        if (node == _lru_head.get()) // head deleting
        {
            if (!node->next.get()) // one element
            {
                _lru_head.reset();
                _lru_tail = nullptr;
            }
            else
            {
                node->next.get()->prev = nullptr;
                _lru_head = std::move(node->next);
            }
        }
        else if (!node->next.get()) // last element
        {
            _lru_tail = node->prev;
            node->prev->next.reset();
        }
        else
        {
            node->next.get()->prev = node->prev;
            node->prev->next = std::move(node->next);
        }
        _cur_size -= node_size; // recalculate memory
        return true;
    }

    bool SimpleLRU::Get(const std::string& key, std::string& value)
    {
        auto node = _lru_index.find(key);
        if (node == _lru_index.end())
        {
            return false;
        }
        value = node->second.get().value;
        node_to_head(&node->second.get());
        return true;
    }

} // namespace Backend
} // namespace Afina

