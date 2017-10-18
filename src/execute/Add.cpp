#include <afina/Storage.h>
#include <afina/execute/Add.h>

#include <iostream>

namespace Afina {
namespace Execute {

// memcached protocol:  "add" means "store this data, but only if the server *doesn't* already
// hold data for this key".
void Add::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::cout << "Add(" << _key << ")" << args << std::endl;
    out = storage.PutIfAbsent(_key, args) ? "STORED" : "NOT_STORED";
}

} // namespace Execute
} // namespace Afina
