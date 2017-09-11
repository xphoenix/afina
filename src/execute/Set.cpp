#include <afina/execute/Set.h>

#include <iostream>

namespace Afina {
namespace Execute {

bool Set::Execute(Storage &storage) {
    std::cout << "Set(" << _key << "): " << std::endl;
    return false;
}

} // namespace Execute
} // namespace Afina
