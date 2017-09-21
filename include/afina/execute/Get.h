#ifndef AFINA_EXECUTE_GET_H
#define AFINA_EXECUTE_GET_H

#include <string>
#include <vector>

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 * # Retrive value for the key
 * Allows to get values for given set of keys
 *
 * After this command, the client expects zero or more items, each of
 * which is received as a text line followed by a data block. After all
 * the items have been transmitted, the server sends the string
 *
 * Each item sent by the server looks like this:
 * VALUE <key> <bytes>\r\n
 * <data>\r\n
 * VALUE ....
 * END
 *
 * Where <key> is the key for the value, <bytes> is the number of bytes in the
 * value and <data> is the value text
 *
 * If some of the keys appearing in a retrieval request are not sent back
 * by the server in the item list this means that the server does not
 * hold items with such keys (because they were never stored, or stored
 * but deleted to make space for more items, or expired, or explicitly
 * deleted by a client).
 */
class Get : public Command {
public:
    Get(const std::vector<std::string> &keys) : _keys(keys) {}
    ~Get() {}

    inline const std::vector<std::string> &keys() const { return _keys; }

    void Execute(Storage &storage, const std::string &args, std::string &out) override;

private:
    std::vector<std::string> _keys;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_GET_H
