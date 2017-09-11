#ifndef AFINA_EXECUTE_SET_H
#define AFINA_EXECUTE_SET_H

#include <cstdint>
#include <string>

#include "InsertCommand.h"

namespace Afina {
namespace Execute {

/**
 *
 *
 */
class Set : public InsertCommand {
public:
    Set(const std::string &key, uint32_t flags, int32_t expire, uint32_t value_size)
        : InsertCommand(key, flags, expire, value_size) {}
    ~Set() {}

    bool Execute(Storage &storage) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_SET_H
