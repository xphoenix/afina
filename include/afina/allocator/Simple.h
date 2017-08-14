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
public:
    Simple(void *base, size_t size);

    /**
     * TODO: semantics
     * @param N size_t
     */
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
};

} // namespace Allocator
} // namespace Afina
#endif // AFINA_ALLOCATOR_SIMPLE_H
