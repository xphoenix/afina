#include <afina/Storage.h>
#include <afina/execute/Replace.h>

#include <iostream>

namespace Afina {
namespace Execute {

// memcached protocol:  "replace" means "store this data, but only if the server *does*
// already hold data for this key".

void Replace::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::cout << "Replace(" << _key << "): " << args << std::endl;
    std::string value;
    if (storage.Get(_key, value)) {
        storage.Set(_key, args);
        out = "STORED";
    } else {
        out = "NOT_STORED";
    }
}

} // namespace Execute
} // namespace Afina
