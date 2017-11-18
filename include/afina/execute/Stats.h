#ifndef AFINA_EXECUTE_STATS_H
#define AFINA_EXECUTE_STATS_H

#include <string>

#include "Command.h"

namespace Afina {
namespace Execute {

class Stats : public Command {
public:
    Stats() {}
    ~Stats() {}
    void Execute(Storage &storage, const std::string &args, std::string &out) override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_STATS_H
