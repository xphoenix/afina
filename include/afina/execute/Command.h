#ifndef AFINA_EXECUTE_COMMAND_H
#define AFINA_EXECUTE_COMMAND_H

#include <string>

namespace Afina {

class Storage;

namespace Execute {

/**
 *
 *
 */
class Command {
public:
    Command() {}
    virtual ~Command() {}

    virtual void Execute(Storage &storage, const std::string &args, std::string &out) = 0;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_COMMAND_H
