#ifndef AFINA_VERSION
#define AFINA_VERSION

#include <string>

namespace Afina {

extern const std::string Version_Major;
extern const std::string Version_Minor;
extern const std::string Version_Patch;
extern const std::string Version_Release;
extern const std::string Version_SHA;

std::string get_version();

} // namespace Afina

#endif // AFINA_VERSION
