#ifndef AFINA_EXECUTE_APPEND_H
#define AFINA_EXECUTE_APPEND_H

#include <cstdint>
#include <string>

#include "InsertCommand.h"

namespace Afina {
namespace Execute {

/**
 *
 *
 */
class Append : public InsertCommand {
public:
    Append(const std::string &key, uint32_t flags, int32_t expire, uint32_t value_size)
        : InsertCommand(key, flags, expire, value_size) {}
    ~Append() {}

    bool Execute(Storage &storage) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_APPEND_H
