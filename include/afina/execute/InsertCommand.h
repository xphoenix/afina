#ifndef AFINA_EXECUTE_INSERT_COMMAND_H
#define AFINA_EXECUTE_INSERT_COMMAND_H

#include <cstdint>
#include <string>

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 * # Basic class for all insert commands
 *
 */
class InsertCommand : public Command {
public:
    InsertCommand(const std::string &key, uint32_t flags, int32_t expire) : _key(key), _flags(flags), _expire(expire) {}
    ~InsertCommand() {}

    inline const std::string &key() const { return _key; }
    inline const uint32_t flags() const { return _flags; }
    inline const int32_t expire() const { return _expire; }

protected:
    const std::string _key;
    const uint32_t _flags;
    const int32_t _expire;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_INSERT_COMMAND_H
