#ifndef AFINA_EXECUTE_GET_H
#define AFINA_EXECUTE_GET_H

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 *
 *
 */
class Get : public Command {
public:
    Get();
    ~Get();

    bool Execute(Storage &storage, std::ostream& out) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_GET_H
