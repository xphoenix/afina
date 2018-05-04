#ifndef AFINA_EXECUTE_DELETE_H
#define AFINA_EXECUTE_DELETE_H

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 * # Remove association for the key
 * Delete existing key from the cache. If key not found then command does
 * nothing
 *
 * Command must write result to the output, which could be:
 * - "DELETED" to indicate success
 * - "NOT_FOUND" to indicate that the item with this key was not found
 */
class Delete : public Command {
private:
    std::string _key;

public:
    bool ExtractArguments(std::string& args_str) override;

    void Execute(Storage &storage, const std::string &args, std::string &out) const override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_DELETE_H
