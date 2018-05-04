#include <afina/execute/KeyValueCommand.h>

namespace Afina {
namespace Execute {

bool KeyValueCommand::ExtractArguments(std::string& args_str) {
	Command::ExtractArguments(args_str); //" noreply"

	std::stringstream sstream(args_str);
	sstream >> _key >> _value;

	if (sstream.fail() || !sstream.eof() || _key.empty() || _value < 0) { return false; }
	else { return true; }
}

} //namespace Execute
} //namespace Afina