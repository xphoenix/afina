#include <afina/Storage.h>
#include <afina/execute/Delete.h>

#include <iostream>

namespace Afina {
namespace Execute {

bool Delete::ExtractArguments(std::string& args_str) {
	Command::ExtractArguments(args_str); //" noreply"

	std::stringstream sstream(args_str);
	sstream >> _key;

	if (sstream.fail() || !sstream.eof() || _key.empty()) { return false; }
	else { return true; }
}

void Delete::Execute(Storage &storage, const std::string& data, std::string &out) const {
	std::cout << "Delete(" << _key << ")" << std::endl;
	out = (storage.Delete(_key) ? "DELETED" : "NOT_FOUND");

	if (_no_reply) { out.clear(); }
}

} //namespace Execute
} //namespace Afina
