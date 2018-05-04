#ifndef AFINA_EXECUTE_COMMAND_H
#define AFINA_EXECUTE_COMMAND_H

#include <string>
#include <sstream>

namespace Afina {

class Storage;

namespace Execute {

/**
 *
 *
 */
class Command {
protected:
    uint32_t _data_size;
    bool _no_reply;

protected:
    Command() : _data_size(0), _no_reply(false) {}

public:
    virtual ~Command() {}

    /*
    	Extracts command arguments from input string. No garantee for its changes. Expected the string without 1)command name and space after it; 2)\r\n
        Returns true if arguments was achived successfully, false in case of parsing error
        
        Realisation of this function in this class extracts " noreply" word, removes it from input string and sets _no_reply flag
    */
    virtual bool ExtractArguments(std::string& args_str) = 0;
    /*
        Returns size of data, required by this command (to be passed as data argument of Execute function)
        0 if not-storage command, <bytes> argument in other cases
    */
    virtual const uint32_t DataSize() const { return _data_size; }

    /*
	data argument should be passed without \r\n (!for compatibility!)
    */
    virtual void Execute(Storage &storage, const std::string& data, std::string &out) const = 0;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_COMMAND_H
