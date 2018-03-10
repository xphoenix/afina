#include <afina/Storage.h>
#include <afina/execute/Set.h>

#include <iostream>
#include <thread>
namespace Afina {
namespace Execute {

// memcached protocol: "set" means "store this data".
void Set::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::cout << "[" << std::this_thread::get_id() << "]" << "Set(" << _key << "): " << args << std::endl;
    storage.Put(_key, args);
    out = "STORED";
}

} // namespace Execute
} // namespace Afina
