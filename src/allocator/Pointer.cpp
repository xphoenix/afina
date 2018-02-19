#include <afina/allocator/Pointer.h>

namespace Afina {
namespace Allocator {

Pointer::Pointer() {}
Pointer::Pointer(const Pointer &) {}
Pointer::Pointer(Pointer &&) {}

Pointer &Pointer::operator=(const Pointer &) { return *this; }
Pointer &Pointer::operator=(Pointer &&) { return *this; }

} // namespace Allocator
} // namespace Afina
