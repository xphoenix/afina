#include <afina/Storage.h>
#include <afina/execute/Set.h>

#include <iostream>

namespace Afina {
namespace Execute {

// memcached protocol: "set" means "store this data".
void Set::Execute(Storage &storage, const std::string &args, std::string &out) const {
	InsertCommand::Execute(storage, args, out); //checks data len

    std::cout << "Set(" << _key << "): " << args << std::endl;
	if (!storage.Put(_key, args)) { out = "NOT_STORED"; }
    else						  { out = "STORED";     }

	if (_no_reply) { out.clear(); }
}

} // namespace Execute
} // namespace Afina
