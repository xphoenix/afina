#ifndef AFINA_EXECUTE_MULTIPLE_STRINGS_H
#define AFINA_EXECUTE_MULTIPLE_STRINGS_H

#include <string>
#include <vector>

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 * Base class for commands with syntax: <command> <strings>*
 */
class MultipleStringsCommand : public Command {
protected:
    MultipleStringsCommand() {}

public:
    inline const std::vector<std::string> &strings() const { return _strings; }

    bool ExtractArguments(std::string& args_str) override;

protected:
    std::vector<std::string> _strings;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_MULTIPLE_STRINGS_H
