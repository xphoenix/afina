#ifndef AFINA_EXECUTE_KEYVALUE_H
#define AFINA_EXECUTE_KEYVALUE_H

#include <cstdint>
#include <string>

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 * # Basic class for all commands with syntax: "<command> <key> <value> [noreply]"
 * Value is number. Example: incr, dect
 */
class KeyValueCommand : public Command {
protected:
    KeyValueCommand() : _value(-1) {}

public:	
    bool ExtractArguments(std::string& args_str) override;

protected:
    std::string _key;
    int64_t _value;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_KEYVALUE_H
