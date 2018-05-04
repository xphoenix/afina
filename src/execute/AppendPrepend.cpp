#include <afina/Storage.h>
#include <afina/execute/AppendPrepend.h>

#include <iostream>

namespace Afina {
namespace Execute {

// memcached protocol: "append" means "add this data to an existing key after existing data".
void AppendPrepend::Execute(Storage &storage, const std::string &args, std::string &out) const {
	InsertCommand::Execute(storage, args, out); //checks data len

	if (_type == true) { std::cout << "Append("  << _key << ")" << args << std::endl; }
	else               { std::cout << "Prepend(" << _key << ")" << args << std::endl; }
    std::string value;
    if (!storage.Get(_key, value)) {
		if (!_no_reply) { out = "NOT_STORED"; }
		else { out.clear(); }
        return;
    }

	std::string result = (_type ? (value + args) : (args + value)); //true - append, false - prepend
	if (!storage.Put(_key, result)) { out = "NOT_STORED"; }
	else							{ out = "STORED";     }

	if (_no_reply) { out.clear(); }
}

} // namespace Execute
} // namespace Afina
