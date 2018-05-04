#ifndef AFINA_EXECUTE_INSERT_COMMAND_H
#define AFINA_EXECUTE_INSERT_COMMAND_H

#include <cstdint>
#include <string>

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 * # Basic class for all insert commands
 * Examples: set, replace, etc.
 */
class InsertCommand : public Command {
protected:
    InsertCommand() : _flags(0), _expire(-1) {}

public:	
    bool ExtractArguments(std::string& args_str) override;
    virtual void Execute(Storage &storage, const std::string& data, std::string &out) const override;	

    inline const std::string &key() const { return _key; }
    inline const uint32_t flags() const { return _flags; }
    inline const int32_t expire() const { return _expire; }

protected:
    std::string _key;

    uint32_t _flags;
    int32_t _expire;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_INSERT_COMMAND_H
