#include <afina/execute/MultipleStringsCommand.h>

#include <sstream>
#include <iostream>

namespace Afina {
namespace Execute {

bool MultipleStringsCommand::ExtractArguments(std::string& args_str) {
	std::stringstream sstream(args_str);
	while (!sstream.eof()) {
		std::string curr_key;
		sstream >> curr_key;

		if (!curr_key.empty()) {
			_strings.push_back(curr_key);
		}
	}

	return true;
}

} // namespace Execute
} // namespace Afina
