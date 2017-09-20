#ifndef AFINA_EXECUTE_COMMAND_H
#define AFINA_EXECUTE_COMMAND_H

#include <iostream>

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

    virtual bool Execute(Storage &storage, std::ostream& out) = 0;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_COMMAND_H
