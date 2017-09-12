#ifndef AFINA_EXECUTE_GET_H
#define AFINA_EXECUTE_GET_H

#include <string>
#include <vector>

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 *
 *
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
