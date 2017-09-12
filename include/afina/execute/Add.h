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
    Add(const std::string &key, uint32_t flags, int32_t expire) : InsertCommand(key, flags, expire) {}
    ~Add() {}

    void Execute(Storage &storage, const std::string &args, std::string &out) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_ADD_H
