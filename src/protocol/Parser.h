#ifndef AFINA_PROTOCOL_PARSER_H
#define AFINA_PROTOCOL_PARSER_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <utility>

#include <afina/execute/Add.h>
#include <afina/execute/Replace.h>
#include <afina/execute/AppendPrepend.h>
#include <afina/execute/IncrDecr.h>
#include <afina/execute/Command.h>
#include <afina/execute/Delete.h>
#include <afina/execute/Get.h>
#include <afina/execute/Set.h>
#include <afina/execute/Stats.h>

#include <cstddef>
#include <cstdint>

namespace Afina {
namespace Protocol {

/**
 * # Memcached protocol parser
 * Parser supports subset of memcached protocol
 */
class Parser {
private:
	typedef std::unique_ptr<Execute::Command> command_ptr;

private:
	// Appends input string to internal buffer (changes whitespace to single space)
	// Extracts until \r\n (includes) is achieved, returns true in this case
	// returns false if \r\n was not found
	// Writes to count_before_space count of symbols in current_str before first space (for extracting of command name)
	// count_before_space = size if space wasn't found and parsing wasn't complete <OR> if _builded_command != nullptr
	// count of symbols in current_str before first space if space was found <OR> len of _current_str without \r\n in if parse complete
	bool _FormatInput(const char *input, const size_t size, size_t& parsed, size_t& count_before_space);

public:
	Parser() { Reset(); }
    /**
     * Push given string into parser input. Method returns true if it was a command parsed out
     * from comulative input. In a such case method Build will return new command
     *
     * @param input sttring to be added to the parsed input
     * @param parsed output parameter tells how many bytes was consumed from the string
     * @return true if command has been parsed out
     */
    bool Parse(const std::string &input, size_t &parsed) { return Parse(input.c_str(), input.size(), parsed); }

    /**
     * Push given string into parser input. Method returns true if it was a command parsed out
     * from comulative input. In a such case method Build will return new command
     *
     * @param input string to be added to the parsed input
     * @param size number of bytes in the input buffer that could be read
     * @param parsed output parameter tells how many bytes was consumed from the string
     * @return true if command has been parsed out
     */
    bool Parse(const char *input, const size_t size, size_t &parsed);

    /**
     * Builds new command from parsed input. In case if it wasn't enough input to parse command out
     * method return nullptr
     */
    command_ptr Build(uint32_t &body_size);

    /**
     * Reset parse so that it could be used to parse out new command
     */
    void Reset();

    inline const std::string &Name() const { return _current_name; }

private:
    std::string _current_str;
    std::string _current_name;
    bool _parse_complete;
    command_ptr _builded_command;

private:
	//Emulates static constructor
	struct _CommandTypes {
		std::vector <std::pair<std::string, std::function<command_ptr()>>> types;
		_CommandTypes();
	};
	static _CommandTypes _command_types; //Calls constructor, where we can register commands

	// Array with pairs <"command name in memcahed protocol", "function, that returns command object for that name">
	//Returns new command_ptr for name. nullptr for unknown command
	static command_ptr _CommandFactory(const std::string& name);
};

} // namespace Protocol
} // namespace Afina

#endif // AFINA_MEMCACHED_PARSER_H
