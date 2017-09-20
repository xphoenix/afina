#ifndef AFINA_EXECUTE_APPEND_H
#define AFINA_EXECUTE_APPEND_H

#include <cstdint>
#include <string>

#include "InsertCommand.h"

namespace Afina {
namespace Execute {

/**
 * # Append data for the key
 * Append new data to the end of value for the given key. If key wasn't found
 * then command does nothing
 *
 * Command must write result to the output, which could be:
 * - "STORED", to indicate success.
 * - "NOT_STORED" to indicate the data was not stored, but not because of an
 * error. This normally means that the condition for the command wasn't met.
 */
class Append : public InsertCommand {
public:
    Append(const std::string &key, uint32_t flags, int32_t expire) : InsertCommand(key, flags, expire) {}
    ~Append() {}

    void Execute(Storage &storage, const std::string &args, std::string &out) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_APPEND_H
