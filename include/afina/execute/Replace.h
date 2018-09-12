#ifndef AFINA_EXECUTE_REPLACE_H
#define AFINA_EXECUTE_REPLACE_H

#include <cstdint>
#include <string>

#include "InsertCommand.h"

namespace Afina {
namespace Execute {

/**
 * # Update existing association
 * Replace value for the key. If key is not present in the cache then command
 * does nothing
 *
 * Command must write result to the output, which could be:
 * - "STORED", to indicate success.
 * - "NOT_STORED" to indicate the data was not stored, but not because of an
 * error. This normally means that the condition for the command wasn't met.
 */
class Replace : public InsertCommand {
public:
    Replace(const std::string &key, uint32_t flags, int32_t expire) : InsertCommand(key, flags, expire) {}
    ~Replace() {}

    void Execute(Storage &storage, const std::string &args, std::string &out) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_REPLACE_H
