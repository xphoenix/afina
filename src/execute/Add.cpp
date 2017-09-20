#include <afina/execute/Add.h>
#include <afina/Storage.h>

#include <iostream>

namespace Afina {
namespace Execute {

void Add::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::cout << "Add(" << _key << ")" << args << std::endl;
    storage.Put(_key, args);
}

} // namespace Execute
} // namespace Afina
