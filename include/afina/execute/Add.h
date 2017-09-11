#ifndef AFINA_EXECUTE_ADD_H
#define AFINA_EXECUTE_ADD_H

#include <cstdint>
#include <string>

#include "InsertCommand.h"

namespace Afina {
namespace Execute {

/**
 *
 *
 */
class Add : public InsertCommand {
public:
    Add(const std::string &key, uint32_t flags, int32_t expire, uint32_t value_size)
        : InsertCommand(key, flags, expire, value_size) {}
    ~Add() {}

    bool Execute(Storage &storage) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_ADD_H
