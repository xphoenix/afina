#ifndef AFINA_ALLOCATOR_POINTER_H
#define AFINA_ALLOCATOR_POINTER_H

namespace Afina {
namespace Allocator {
// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Simple;

class Pointer {

private:
	void* point;//point points to the place in the storage of pointers
public:
    Pointer(){
    	point=nullptr;
    }
	Pointer(const Pointer &p);
	Pointer(Pointer &&p);
	Pointer &operator=(const Pointer &p);
	Pointer &operator=(Pointer &&p);
    void *get() const{
    	if (point!=nullptr)
    		return (*(void**)point);
    	else
    		return nullptr;
    }
    void set(void* p){
    	point=p;
    }
};

} // namespace Allocator
} // namespace Afina

#endif // AFINA_ALLOCATOR_POINTER_H
