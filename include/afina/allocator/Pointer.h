//Pointer.h - интерфейс "умного" указателя, которые возвращает аллокатор.
// Фактически, это обертка над указателем нужная для поддержки дефрагментации

#ifndef AFINA_ALLOCATOR_POINTER_H
#define AFINA_ALLOCATOR_POINTER_H
#include <cstddef>
#include <iostream>
#include <set>
#include <cstring>

using namespace std;

namespace Afina {
namespace Allocator {
// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Simple;

struct point{
    void* first;
    void* last;
    point* next;
    point* prev;
};

class Pointer {
public:
    Pointer();
    Pointer(point *new_ptr);
    ~Pointer(){};

    void *get() const;
    point *get_ptr() const;
    size_t get_size();
    void free();

private:
    point* ptr;
};

} // namespace Allocator
} // namespace Afina

#endif // AFINA_ALLOCATOR_POINTER_H
