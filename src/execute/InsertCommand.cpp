#include <afina/execute/InsertCommand.h>

namespace Afina {
namespace Execute {

bool InsertCommand::ExtractArguments(std::string& args_str) {
	Command::ExtractArguments(args_str); //" noreply"

	std::stringstream sstream(args_str);
	sstream >> _key >> _flags >> _expire >> _data_size;

	if (sstream.fail() || !sstream.eof() || _key.empty()) { return false; }
	else { return true; }
}

void InsertCommand::Execute(Storage &storage, const std::string& data, std::string &out) const {
	if (data.size() != _data_size) {
		throw std::runtime_error("Wrong len of data in Execute command!");
	}
}

} //namespace Execute
} //namespace Afina