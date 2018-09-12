#ifndef AFINA_LOGGING_SERVICE_H
#define AFINA_LOGGING_SERVICE_H

#include <map>
#include <memory>
#include <string>

#include <spdlog/logger.h>

namespace Afina {
namespace Logging {

/**
 * # Provides loggers for rest of the system
 *
 */
class Service {
public:
    virtual ~Service() {}

    virtual void Start() = 0;

    virtual void Stop() = 0;

    virtual std::shared_ptr<spdlog::logger> select(const std::string &name) noexcept = 0;

    virtual std::unique_ptr<spdlog::logger> create(const std::string &name,
                                                   const std::map<std::string, std::string> &mdc) noexcept = 0;

    virtual void reopen_all() = 0;
};

} // namespace Logging
} // namespace Afina

#endif // AFINA_LOGGING_SERVICE_H
