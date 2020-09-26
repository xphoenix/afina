#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
    bool result;

    std::string k = key;
    std::reference_wrapper<std::string> a(k);

    // вызываем PutIfAbsent или Set в зависимости от того,
    // есть ли нужный ключ в двусвязном списке
    if (_lru_index.find(a) == _lru_index.end())
    {

        // размер нового элемента
        std::size_t size_of_new = key.size() + value.size();

        // влезет вообще или нет
        if (size_of_new > _max_size)
        {
            return false;
        }

        // влезет или нет с учетом уже имеющихся
        if (size_of_new > _max_size - _current_size)
        {
            // удаляем последние, пока не влезет
            while (size_of_new > _max_size - _current_size)
            {
                const std::string k = _last_node->key;
                this->Delete(k);
            }
        }

        result = PutIfAbsent(key, value);
    }
    else
    {
        result = Set(key, value);
    }

    return result;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    std::string *k = new std::string(key);
    std::reference_wrapper<std::string> a(*k);

    // если ключ уже есть, возвращаем false
    if (_lru_index.find(a) != _lru_index.end())
    {
        delete k;
        return false;
    }

    // создаем новую структуру-вершину и инициализируем ее
    std::unique_ptr<lru_node> new_node_ptr(new lru_node(key));
    std::reference_wrapper<lru_node> b(*new_node_ptr.get());

    b.get().value = value;
    b.get().key_pointer_in_tree = k;

    // если уже есть другие элементы:
    if (_lru_head)
    {
        (_lru_head.get())->next = new_node_ptr.get();
        b.get().prev = std::move(_lru_head);
    }
    // других элементов нет - этот первый:
    else
    {
        _last_node = &b.get();
    }

    _lru_head = std::move(new_node_ptr);

    // добавляем пару (ключ, структура) в дерево
    _lru_index.insert({a, b});

    _current_size += key.size() + value.size();

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    std::string k = key;
    std::reference_wrapper<std::string> a(k);

    // если ключа нет, возвращаем false
    if (_lru_index.find(a) == _lru_index.end())
    {
        return false;
    }

    // переносим элемент в начало двусвязного списка:

    // "удаляем" его из середины списка
    // если это головная вершина, то ничего не переставляем
    if (_lru_head.get() != &_lru_index.at(a).get())
    {
        // (если это не головная структура, то у нее есть next)
        // если у следующего за рассматриваемым нет следующего, то
        // им станет рассматриваемый
        if (!(_lru_index.at(a).get().next)->next)
        {
            (_lru_index.at(a).get().next)->next = (_lru_index.at(a).get().next)->prev.get();
        }

        // нельзя терять прошлый элемент
        if (_lru_index.at(a).get().prev.get())
        {
            (_lru_index.at(a).get().prev.get())->next = _lru_index.at(a).get().next;
        }
        // если нет прошлого, то этот сейчас последний, и
        // тогда последним станет следующий за ним, а он есть
        // так как мы в этом if'е => делаем его последним
        else
        {
            _last_node = _lru_index.at(a).get().next;
        }

        // создаем временный умный указатель для осуществления перестановки
        std::unique_ptr<lru_node> tmp_ptr;
        tmp_ptr = std::move(_lru_index.at(a).get().prev);

        // если это не головная вершина, то у нее есть next
        _lru_index.at(a).get().prev = std::move(_lru_head);
        _lru_head = std::move((_lru_index.at(a).get().next)->prev);
        (_lru_index.at(a).get().next)->prev = std::move(tmp_ptr);

        // у головной вершины не может быть следующей
        _lru_index.at(a).get().next = nullptr;
    }

    _current_size -= _lru_index.at(a).get().value.size();
    _lru_index.at(a).get().value = value;
    _current_size += value.size();

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
    std::string k = key;
    std::reference_wrapper<std::string> a(k);
    std::unique_ptr<lru_node> current_block_ptr;

    // если ключа нет, возвращаем false
    if (_lru_index.find(a) == _lru_index.end())
    {
        return false;
    }

    // чтобы не потерять unique_ptr на удаляемую структуру
    // в ходе перестановок указателей, временно сохраним его
    if (_lru_index.at(a).get().next)
    {
        current_block_ptr = std::move(_lru_index.at(a).get().next->prev);
    }
    else
    {
        current_block_ptr = std::move(_lru_head);
    }

    // "удаляем" из середины списка:

    if (_lru_index.at(a).get().prev.get())
    {
        (_lru_index.at(a).get().prev)->next = _lru_index.at(a).get().next;
    }
    // если это последний, то ставим указатель на следующий элемент в качестве
    // последнего (не важно nullptr это или нет)
    else
    {
        if (_last_node == &_lru_index.at(a).get())
        {
            (_lru_index.at(a).get().next)->prev = std::move(_lru_index.at(a).get().prev);
        }

        _last_node = _lru_index.at(a).get().next;
    }

    if (_lru_index.at(a).get().next)
    {
        if (_lru_index.at(a).get().prev.get())
        {
            (_lru_index.at(a).get().next)->prev = std::move(_lru_index.at(a).get().prev);
        }
    }
    else
    {
        if (_lru_index.at(a).get().prev.get())
        {
            _lru_head = std::move(_lru_index.at(a).get().prev);
        }
    }

    std::string *key_ptr = _lru_index.at(a).get().key_pointer_in_tree;

    _current_size -= key.size() + _lru_index.at(a).get().value.size();

    // удаляем из дерева
    _lru_index.erase(a);

    // очищаем память
    delete key_ptr;

    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value)
{
    std::string k = key;
    std::reference_wrapper<std::string> a(k);

    // если ключа нет, возвращаем false
    if (_lru_index.find(a) == _lru_index.end())
    {
        return false;
    }

    value = _lru_index.at(a).get().value;

    return true;
}

} // namespace Backend
} // namespace Afina
