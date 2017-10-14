#ifndef AFINA_ALLOCATOR_POINTER_H
#define AFINA_ALLOCATOR_POINTER_H

#include <cstddef>
#include <iostream>
#include <set>
#include <cstring>

namespace Afina {
namespace Allocator {
// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Simple;
    
struct block{
    void* first;
    void* last;
    block* next;
    block* prev;
};
    

class Pointer {
public:
    Pointer();
    
   // Pointer(const Pointer &);
   // Pointer(Pointer &&);
    //Pointer(Pointer)
    Pointer(block *new_ptr);
    void *get() const;
    size_t get_size();
    block *get_ptr() const{
        return ptr;
    }

   // Pointer &operator=(const Pointer &);
    //Pointer &operator=(Pointer &&);

private:
    block* ptr;
};

} // namespace Allocator
} // namespace Afina

#endif // AFINA_ALLOCATOR_POINTER_H
