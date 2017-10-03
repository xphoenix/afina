#include <afina/allocator/Pointer.h>

namespace Afina {
namespace Allocator {


Pointer::Pointer(const Pointer &p) { point=p.point;}
Pointer::Pointer(Pointer &&p) {point=p.point;}

Pointer &Pointer::operator=(const Pointer &p) { point=p.point;return *this; }
Pointer &Pointer::operator=(Pointer &&p) {point=p.point; return *this; }
} // namespace Allocator
} // namespace Afina
