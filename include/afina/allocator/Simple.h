#ifndef AFINA_ALLOCATOR_SIMPLE_H
#define AFINA_ALLOCATOR_SIMPLE_H

#include <string>
#include <cstddef>
#include <afina/allocator/Pointer.h>

namespace Afina {
namespace Allocator {

// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Pointer;

/**
 * Wraps given memory area and provides defagmentation allocator interface on
 * the top of it.
 *
 * Allocator instance doesn't take ownership of wrapped memmory and do not delete it
 * on destruction. So caller must take care of resource cleaup after allocator stop
 * being needs
 */
// TODO: Implements interface to allow usage as C++ allocators
class Simple {
public:
    Simple(void *base, const size_t size);

    /**
     * TODO: semantics
     * @param N size_t
     */
    
    Pointer alloc_empty(size_t N);
    Pointer alloc_in_fspace(size_t N);
    Pointer find_hole_space(size_t N);
    Pointer alloc(size_t N);

    /**
     * TODO: semantics
     * @param p Pointer
     * @param N size_t
     */
    void realloc(Pointer &p, size_t N);

    /**
     * TODO: semantics
     * @param p Pointer
     */
    void free(Pointer &p);

    /**
     * TODO: semantics
     */
    void defrag();

    /**
     * TODO: semantics
     */
    std::string dump() const;

private:
    block* free_space;
    block buffer;
    block* root; //pointer to the end of buffer = the begin of list
    block* last_elmnt_lst;
    bool is_empty;
    void *_base;
    const size_t _base_len;
};

} // namespace Allocator
} // namespace Afina
#endif // AFINA_ALLOCATOR_SIMPLE_H
