#include <afina/allocator/Pointer.h>

namespace Afina {
namespace Allocator {
    
    

Pointer::Pointer() {
    ptr = nullptr;
}
Pointer::Pointer(block* n_ptr){
    ptr = n_ptr;
}

void* Pointer::get() const {
    if (ptr == nullptr) {
        return nullptr;
    }
    return ptr->first;
}
    
size_t Pointer::get_size(){
    if(ptr == nullptr)
        return 0;
    if(ptr->first == nullptr)
        return 0;
    
    return static_cast<char *>(ptr->last) - static_cast<char *>(ptr->first);
}
//Pointer::Pointer(const Pointer &) {}
//Pointer::Pointer(Pointer &&) {}

//Pointer &Pointer::operator=(const Pointer &) { return *this; }
//Pointer &Pointer::operator=(Pointer &&) { return *this; }

} // namespace Allocator
} // namespace Afina
