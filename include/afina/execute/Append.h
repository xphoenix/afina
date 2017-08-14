#ifndef AFINA_EXECUTE_APPEND_H
#define AFINA_EXECUTE_APPEND_H

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 *
 *
 */
class Append : public Command {
public:
    Append();
    ~Append();

    bool Execute(Storage &storage) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_APPEND_H
