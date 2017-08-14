#ifndef AFINA_EXECUTE_ADD_H
#define AFINA_EXECUTE_ADD_H

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 *
 *
 */
class Add : public Command {
public:
    Add();
    ~Add();

    bool Execute(Storage &storage) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_ADD_H
