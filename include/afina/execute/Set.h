#ifndef AFINA_EXECUTE_SET_H
#define AFINA_EXECUTE_SET_H

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 *
 *
 */
class Set : public Command {
public:
    Set();
    ~Set();

    bool Execute(Storage &storage) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_SET_H
