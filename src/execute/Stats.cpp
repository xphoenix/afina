#include <afina/Storage.h>
#include <afina/execute/Stats.h>

#include <iostream>
#include <iterator>
#include <sstream>

namespace Afina {
namespace Execute {

void Stats::Execute(Storage &storage, const std::string &args, std::string &out) { out.assign("END"); }

} // namespace Execute
} // namespace Afina
