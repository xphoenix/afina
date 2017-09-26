#include <afina/allocator/Simple.h>

#include <afina/allocator/Pointer.h>
#include <afina/allocator/Error.h>

namespace Afina {
namespace Allocator {


Simple::Simple(void *base, size_t base_size) {
    //init
    buff.first = base;
    buff.last = static_cast<char *>(base) + base_size - sizeof(point) ;
    root = static_cast<point *>(buff.last);
    root->prev = nullptr;
    root->next  = nullptr;
    root->first = nullptr;
    root->last  = nullptr;
    last_node = root;
    free_points = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Pointer Simple::_alloc_root(size_t N) {
        //Выделить кусок памяти запрошенного размера. Если это невозможно, запрошенно больше чем есть свободной памяти,
        // то бросить исключение.
        // Если памяти достаточно, но она фрагментированна - выполнить дефрагментацию и потом выделить.
        root->first = buff.first;
        root->last = static_cast<void *>(static_cast<char*>(buff.first) + N);//преобразование указателя
                                                    // на базовый класс в указатель на производный класс.
        return Pointer(root); }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Pointer Simple::_alloc_last_node(size_t N){
    if (free_points != nullptr) {
        point *new_free_point = free_points->prev;
        free_points->prev = root; // указатель на следующий из предыдущего = текущий
        free_points->first = root->last; // начало новой строки - конец предыдущей
        root->next = free_points;
        free_points->last = static_cast<void *>(static_cast<char *>(root->last) + N);
        root = free_points;
        free_points = new_free_point;
        root->next = nullptr;
    }
    else{
        last_node--;
        root->next = last_node; // добавили новый элемент
        last_node->prev = root; // указатель на следующий из предыдущего = текущий
        last_node->first = root->last; // начало новой строки - конец предыдущей
        last_node->last = static_cast<void*>(static_cast<char*>(root->last) + N);
        last_node->next = nullptr; // следующего пока нет
        root = last_node; // last_node = новая верхушка
    }
    return Pointer(root);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Pointer Simple::_alloc_find_free_place(size_t N){

        for(point *cur = static_cast<point *>(buff.last); cur->next != nullptr; cur = cur->next){
            if((cur->next)->first != nullptr){
                size_t sz = static_cast<char*>((cur->next)->first) - static_cast<char*>(cur->last); // размер кусочка
                if(sz >= N){
                    if (free_points != nullptr){
                        point *new_free_points = free_points->prev;
                        free_points->next = cur->next;
                        free_points->prev = cur; // указатель на следующий из предыдущего = текущий
                        free_points->first = cur->last; // начало новой строки - конец предыдущей
                        free_points->last = static_cast<void*>(static_cast<char*>(cur->last) + N);

                        (cur->next)->prev = free_points;
                        point *new_one = free_points;
                        free_points = new_free_points;
                        return Pointer(new_one);
                    }
                    else{
                        last_node--;
                        last_node->prev = cur;
                        last_node->next = cur->next;
                        last_node->first = cur->last;
                        last_node->last = static_cast<void*>(static_cast<char*>(cur->last) + N);
                        (cur->next)->prev = last_node;
                        cur->next = last_node;
                        return Pointer(last_node);
                    }
                }
            }
        }
        throw AllocError(AllocErrorType::NoMemory, "Try defraq");
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* выделяем память размера N */
    Pointer Simple::alloc(size_t N) {

        // Память в буфере ещё не выделялась
        // просто создаём нулевой элемент в корне root
        if (root->prev  == nullptr &&
            root->next  == nullptr &&
            root->first == nullptr &&
            root->last  == nullptr &&
            N + sizeof(point) < static_cast<char*>(buff.last) - static_cast<char*>(buff.first))
            return _alloc_root(N);

        // добавляем в конец списка
        if (last_node > static_cast<void*>(static_cast<char*>(last_node->last) + N + sizeof(point)))
            return _alloc_last_node(N);


        // посмотрим, есть ли место в дырках
        return _alloc_find_free_place(N);
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Simple::defrag() {
        int i = 0;
        for(point *cur = static_cast<point*>(buff.last)->next; cur != nullptr; cur = cur->next){
            if(cur->first != nullptr) {
                if (cur->first > cur->prev->last) {
                    size_t sz = static_cast<char*>(cur->last) - static_cast<char*>(cur->first);
                    memcpy(cur->prev->last, cur->first, sz);
                    cur->first = cur->prev->last;
                    cur->last = static_cast<char*>(cur->first) + sz;
                }
            }
            else{
                cur->last = cur->prev->last;
            }
        }
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Simple::realloc(Pointer &p, size_t N) {
        //Изменить размер выделенной ранее области, уменьшить или увеличить.
        // При этом нужно, по возможности, избегать копирования блоков памяти.
        void *info_link = p.get();
        size_t size = p.get_size();
        free(p);
        p = alloc(N);
        memcpy(p.get(), info_link, size);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void Simple::free(Pointer& p){
            point *prev = nullptr;
            point *get = p.get_ptr();

            if (get != nullptr){
                // Если free_points пока не существует
                if (free_points == nullptr){
                    free_points = get;
                    prev = nullptr;
                }
                else{
                    // Если существует, то создадим новый
                    free_points->next = get;
                    prev = free_points;
                    free_points = free_points->next;
                }


                // Если элемент не последний
                if (free_points->next != nullptr){

                    // если не в начале
                    if (free_points->prev != nullptr){

                        // меняем местами
                        point *next = free_points->next;
                        point *last = free_points->prev;
                        next->prev = last;
                        last->next = next;

                    }
                    else{
                        // если он первый,
                        // удаляем предыдущий элемент у следующего
                        //cout << "free_points->prev == nullptr\n";
                        (free_points->next)->prev = nullptr;
                    }
                }
                else{
                    //cout << "free_points->next == nullptr\n";
                    //cout << "MUST BE ROOT\n";
                    // если последний, значит он root!
                    if (free_points->prev != nullptr){

                        // если последний, но не первый,
                        // то смещаем root назад
                        //cout << "free_points->prev != nullptr\n";
                        root = root->prev;
                        root->next = nullptr;

                        //free_points уже не равен root
                    }
                    else{
                        //если он последний и первый, то он просто root
                        // и больше ничего нет
                        // удалим его

                        root->next = nullptr;
                        root->last = nullptr;
                        root->first = nullptr;
                        root->last = nullptr;
                    }
                }


                if (root != free_points){
                    free_points->first = nullptr;
                    free_points->last = nullptr;
                    free_points->prev = prev;
                    free_points->next = nullptr;
                }
                else{
                    // если попали сюда, то уже удлалили вообще всё
                    free_points = prev;
                    if (free_points != nullptr)
                        free_points->next = nullptr;
                }
            }

            p = nullptr;
}


//std::string Simple::dump() const { return ""; }

} // namespace Allocator
} // namespace Afina
