#ifndef AFINA_LOGGING_SERVICE_IMPL_H
#define AFINA_LOGGING_SERVICE_IMPL_H

#include <afina/logging/Config.h>
#include <afina/logging/Service.h>

namespace Afina {
namespace Logging {

/**
 * # Provides loggers for rest of the system
 *
 */
class ServiceImpl : public Service {
public:
    explicit ServiceImpl(std::shared_ptr<Config> cfg);
    ~ServiceImpl() override;

    // See Service.h
    void Start() override;

    // See Service.h
    void Stop() override;

    // See Service.h
    std::shared_ptr<spdlog::logger> select(const std::string &name) noexcept override;

    // See Service.h
    std::unique_ptr<spdlog::logger> create(const std::string &name,
                                           const std::map<std::string, std::string> &mdc) noexcept override;

    // See Service.h
    void reopen_all() override;

private:
    std::shared_ptr<Config> _cfg;

    // TODO: bug: if service not started all select return _root, which is nullptr
    std::shared_ptr<spdlog::logger> _root;
};

} // namespace Logging
} // namespace Afina
#endif // AFINA_LOGGING_SERVICE_IMPL_H
