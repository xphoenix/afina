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
    InsertCommand(const std::string &key, uint32_t flags, int32_t expire, uint32_t value_size)
        : _key(key), _flags(flags), _expire(expire), _value_size(value_size) {}
    ~InsertCommand() {}

    inline const std::string &key() const { return _key; }
    inline const uint32_t flags() const { return _flags; }
    inline const int32_t expire() const { return _expire; }
    inline const uint32_t valueSize() const { return _value_size; }

protected:
    const std::string _key;
    const uint32_t _flags;
    const int32_t _expire;
    const uint32_t _value_size;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_INSERT_COMMAND_H
