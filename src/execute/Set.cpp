#include <afina/execute/Set.h>

#include <iostream>

namespace Afina {
namespace Execute {

void Set::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::cout << "Set(" << _key << "): " << args << std::endl;
    out = "STORED";
}

} // namespace Execute
} // namespace Afina
