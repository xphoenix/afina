#include <afina/Storage.h>
#include <afina/execute/Replace.h>

#include <iostream>

namespace Afina {
namespace Execute {

// memcached protocol:  "replace" means "store this data, but only if the server *does*
// already hold data for this key".

void Replace::Execute(Storage &storage, const std::string &args, std::string &out) const {
	InsertCommand::Execute(storage, args, out); //checks data len

    std::cout << "Replace(" << _key << "): " << args << std::endl;
    std::string value;
    if (!storage.Get(_key, value)) {
		if (!_no_reply) { out = "NOT_STORED"; }
		else { out.clear(); }
		return;
	}
    if (!storage.Set(_key, args)) { out = "NOT_STORED"; }
    else			  { out = "STORED";     }

    if (_no_reply) { out.clear(); }
}

} // namespace Execute
} // namespace Afina
