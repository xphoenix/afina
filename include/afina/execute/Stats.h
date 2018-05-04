#ifndef AFINA_EXECUTE_STATS_H
#define AFINA_EXECUTE_STATS_H

#include "MultipleStringsCommand.h"

namespace Afina {
namespace Execute {

class Stats : public MultipleStringsCommand {
public:
    void Execute(Storage &storage, const std::string &args, std::string &out) const override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_STATS_H
