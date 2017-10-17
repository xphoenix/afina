#include <afina/Storage.h>
#include <afina/execute/Append.h>

#include <iostream>

namespace Afina {
namespace Execute {

// memcached protocol: "append" means "add this data to an existing key after existing data".
void Append::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::cout << "Append(" << _key << ")" << args << std::endl;
    std::string value;
    if (!storage.Get(_key, value)) {
        out.assign("NOT_STORED");
        return;
    }
    storage.Put(_key, value + args);
    out.assign("STORED");
}

} // namespace Execute
} // namespace Afina
