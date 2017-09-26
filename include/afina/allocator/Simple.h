#ifndef AFINA_ALLOCATOR_SIMPLE_H
#define AFINA_ALLOCATOR_SIMPLE_H

#include <string>

namespace Afina {
namespace Allocator {

// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Pointer;

/**
 * Wraps given memory area and provides defagmentation allocator interface on
 * the top of it.
 */
// TODO: Implements interface to allow usage as C++ allocators
class Simple {
private:
	int renew_empty_slot();
	void * nearest_free_block(void* block);
	void* give_pointer(void * block);
	void* occupy_block(void* block);
	void* slice_block(void* block,size_t N);
	void shrink_block(void* block,size_t N);
	void* find_to_alloc(size_t N);
	void clean_block(void *block);
	void create_free_block(void* block);
	
	
public:
	void *base;
	size_t size;
	int space;//how much space for data we actually have (service needs aside)
	void* empty_slot;// empty slot in the storage of pointers
	int pointers_reserved;//how much space we have for pointers storage
	void* curr_block;//the block to start finding free blocks to suffice allocation from
	void* start;//the beginning of memory used for allocation
    Simple(void *base, size_t size);
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);
    void defrag();
    std::string dump() const;
};

} // namespace Allocator
} // namespace Afina
#endif // AFINA_ALLOCATOR_SIMPLE_H
