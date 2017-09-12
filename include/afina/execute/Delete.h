#ifndef AFINA_EXECUTE_DELETE_H
#define AFINA_EXECUTE_DELETE_H

#include "Command.h"

namespace Afina {
namespace Execute {

/**
 *
 *
 */
class Delete : public Command {
public:
    Delete();
    ~Delete();

    void Execute(Storage &storage, const std::string &args, std::string &out) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_DELETE_H
