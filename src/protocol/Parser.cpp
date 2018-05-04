#include "Parser.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace Afina {
namespace Protocol {

Parser::_CommandTypes Parser::_command_types;

Parser::_CommandTypes::_CommandTypes() {
	types.push_back(std::make_pair("set",     []() { return std::unique_ptr<Execute::Command>(new Execute::Set);                  }));
	types.push_back(std::make_pair("replace", []() { return std::unique_ptr<Execute::Command>(new Execute::Replace);              }));
	types.push_back(std::make_pair("append",  []() { return std::unique_ptr<Execute::Command>(new Execute::AppendPrepend(true));  }));
	types.push_back(std::make_pair("prepend", []() { return std::unique_ptr<Execute::Command>(new Execute::AppendPrepend(false)); }));
	types.push_back(std::make_pair("add",     []() { return std::unique_ptr<Execute::Command>(new Execute::Add);                  }));

	types.push_back(std::make_pair("incr", []() { return std::unique_ptr<Execute::Command>(new Execute::IncrDecr(true));  }));
	types.push_back(std::make_pair("decr", []() { return std::unique_ptr<Execute::Command>(new Execute::IncrDecr(false)); }));

	types.push_back(std::make_pair("get",  []() { return std::unique_ptr<Execute::Command>(new Execute::Get); }));
	types.push_back(std::make_pair("gets", []() { return std::unique_ptr<Execute::Command>(new Execute::Get); }));

	types.push_back(std::make_pair("delete", []() { return std::unique_ptr<Execute::Command>(new Execute::Delete); }));

	types.push_back(std::make_pair("stats", []() { return std::unique_ptr<Execute::Command>(new Execute::Stats); }));
}

Parser::command_ptr Parser::_CommandFactory(const std::string& name) {
	for (auto it : _command_types.types) {
		if (it.first == name) { return it.second(); }
	}
	return nullptr;
}

bool Parser::_FormatInput(const char *input, const size_t size, size_t& parsed, size_t& count_before_space) {
	count_before_space = size;
	bool is_whitespace = false;

	for (parsed = 0; parsed < size; parsed++) {
		if (input[parsed] == '\n') {
			if (!_current_str.empty() && _current_str[_current_str.size() - 1] == '\r') {
				if (count_before_space == size && _builded_command == nullptr) { count_before_space = _current_str.size() - 1; } //No spaces, all is name

				_current_str += '\n';	
				++parsed; //for \n
				return true;
			}
		}

		if (input[parsed] == ' ' || input[parsed] == '\t') {
			if (!is_whitespace) {
				if (count_before_space == size && _builded_command == nullptr) { count_before_space = _current_str.size(); }
				_current_str += ' ';
			}
			is_whitespace = true;
		}
		else {
			is_whitespace = false;
			_current_str += input[parsed];
		}
	}

	return false;
}

// See Parse.h
bool Parser::Parse(const char *input, const size_t size, size_t &parsed) {
	size_t count_before_space = size;
	bool parse_complete = _FormatInput(input, size, parsed, count_before_space);
	
	if (count_before_space != size) { //a new command should be extracted
		_current_name = _current_str.substr(0, count_before_space);
		_builded_command = _CommandFactory(_current_name);

		if (_current_str[count_before_space] == ' ') { _current_str = _current_str.substr(count_before_space + 1); } //For space case
		else { _current_str = _current_str.substr(count_before_space); } //\r\n after the name of command
	}
	if (parse_complete) {
		if (_builded_command == nullptr) { throw std::runtime_error("Unknown command name!"); }
		
		_current_str = _current_str.substr(0, _current_str.size() - 2); //Removes \r\n
		if (!_builded_command->ExtractArguments(_current_str)) { throw std::runtime_error("Wrong command args!"); }
	}

	return parse_complete;
}

// See Parse.h
std::unique_ptr<Execute::Command> Parser::Build(uint32_t &body_size) {
	body_size = _builded_command->DataSize();
	return std::move(_builded_command);
}

// See Parse.h
void Parser::Reset() {
	_parse_complete = false;
	_current_str.clear();
	_builded_command.reset();
	_current_name.clear();
}

} // namespace Protocol
} // namespace Afina
