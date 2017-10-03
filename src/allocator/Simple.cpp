#include <afina/allocator/Simple.h>

#include <afina/allocator/Pointer.h>

namespace Afina {
namespace Allocator {

Simple::Simple(void *base, size_t size) : _base(base), _base_len(size) {}

/**
 * TODO: semantics
 * @param N size_t
 */
Pointer Simple::alloc(size_t N) { return Pointer(); }

/**
 * TODO: semantics
 * @param p Pointer
 * @param N size_t
 */
void Simple::realloc(Pointer &p, size_t N) {}

/**
 * TODO: semantics
 * @param p Pointer
 */
void Simple::free(Pointer &p) {}

/**
 * TODO: semantics
 */
void Simple::defrag() {}

/**
 * TODO: semantics
 */
std::string Simple::dump() const { return ""; }

} // namespace Allocator
} // namespace Afina
