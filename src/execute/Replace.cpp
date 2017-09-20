#include <afina/Storage.h>
#include <afina/execute/Replace.h>

#include <iostream>

namespace Afina {
namespace Execute {

void Replace::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::cout << "Replace(" << _key << "): " << args << std::endl;
    out = "NOT_STORED";
}

} // namespace Execute
} // namespace Afina
