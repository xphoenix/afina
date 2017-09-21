#include <afina/execute/Set.h>
#include <afina/Storage.h>

#include <iostream>

namespace Afina {
namespace Execute {

void Set::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::cout << "Set(" << _key << "): " << args << std::endl;

    std::string value;
    if (storage.Get(_key, value))
    {
        storage.Put(_key, args);
        out = "STORED";
    } else {
        out = "NOT_STORED";
    }
}

} // namespace Execute
} // namespace Afina
