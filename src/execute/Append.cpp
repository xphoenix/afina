#include <afina/execute/Append.h>
#include <afina/Storage.h>

#include <iostream>

namespace Afina {
namespace Execute {

void Append::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::cout << "Append(" << _key << ")" << args << std::endl;
    std::string value;
    storage.Get(_key, value);
    out = storage.Put(_key, value + args);
}

} // namespace Execute
} // namespace Afina
