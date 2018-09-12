#include "ServiceImpl.h"

#include <regex>
#include <sstream>
#include <unordered_set>
#include <utility>

#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/file_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <afina/logging/Config.h>
#include <afina/logging/Service.h>

namespace Afina {
namespace Logging {

///////////////////////////////////////////////////////////////////////////////////////////
// File sync that could be reopened at any time
///////////////////////////////////////////////////////////////////////////////////////////
template <class Mutex> class ext_rotate_file_sink : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit ext_rotate_file_sink(const spdlog::filename_t &filename, bool truncate = false) : _force_flush(false) {
        _file_helper.open(filename, truncate);
    }
    void flush() override { _file_helper.flush(); }
    void set_force_flush(bool force_flush) { _force_flush = force_flush; }
    void reopen() {
        SPDLOG_DEBUG("Reopen: " + _file_helper.filename());
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::_mutex);
        _file_helper.reopen(false);
    }

protected:
    void _sink_it(const spdlog::details::log_msg &msg) override {
        _file_helper.write(msg);
        if (_force_flush)
            _file_helper.flush();
    }

private:
    spdlog::details::file_helper _file_helper;
    bool _force_flush;
};
using ext_rotate_file_sink_mt = ext_rotate_file_sink<std::mutex>;
using ext_rotate_file_sink_st = ext_rotate_file_sink<spdlog::details::null_mutex>;

///////////////////////////////////////////////////////////////////////////////////////////
// Extension of spdlog's distribution sink which allows to get access to all internal sinks
///////////////////////////////////////////////////////////////////////////////////////////
template <class Mutex> class dist_sink : public spdlog::sinks::dist_sink<Mutex> {
public:
    const std::vector<spdlog::sink_ptr> &sinks() const { return spdlog::sinks::dist_sink<Mutex>::_sinks; }
};
using dist_sink_mt = dist_sink<std::mutex>; // TODO: may be use it?
using dist_sink_st = dist_sink<spdlog::details::null_mutex>;

// See ServiceImpl.h
ServiceImpl::ServiceImpl(std::shared_ptr<Config> cfg) : _cfg(std::move(cfg)), _root() {}

// See ServiceImpl.h
ServiceImpl::~ServiceImpl() = default;

// See ServiceImpl.h
void ServiceImpl::Start() {
    // Setup spdlog
    spdlog::set_async_mode(512, spdlog::async_overflow_policy::block_retry, nullptr, std::chrono::seconds(2));

    // First build appenders
    std::map<std::string, spdlog::sink_ptr> results;
    for (auto it = _cfg->appenders.begin(); it != _cfg->appenders.end(); ++it) {
        // Source
        std::string name = it->first;
        Appender &pAppender = it->second;

        // Convert
        spdlog::sink_ptr ptr = nullptr;
        switch (pAppender.type) {
        case Appender::Type::STDOUT: {
            ptr = spdlog::sinks::stdout_sink_mt::instance();
            if (pAppender.color) {
                ptr = std::make_shared<spdlog::sinks::ansicolor_sink>(ptr);
            }
            break;
        }

        case Appender::Type::STDERR: {
            ptr = spdlog::sinks::stderr_sink_mt::instance();
            if (pAppender.color) {
                ptr = std::make_shared<spdlog::sinks::ansicolor_sink>(ptr);
            }
            break;
        }

        case Appender::Type::FILE: {
            ptr = std::make_shared<ext_rotate_file_sink_mt>(pAppender.file);
            break;
        }

        case Appender::Type::DAILY: {
            // TODO: from config
            ptr = std::make_shared<spdlog::sinks::daily_file_sink_mt>(pAppender.file, pAppender.rotate_at_hours,
                                                                      pAppender.rotate_at_mins);
            break;
        }

        case Appender::Type::SIZED: {
            // TODO: from config
            ptr = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(pAppender.file, pAppender.rotate_at_size,
                                                                         pAppender.history_to_keep);
            break;
        }

        case Appender::Type::SYSLOG: {
            ptr = std::make_shared<spdlog::sinks::syslog_sink>(pAppender.ident, pAppender.option, pAppender.facility);
            break;
        }

        default:
            throw std::runtime_error("Invalid appender type");
        }
        results[name] = ptr;
    }

    // Create and register loggers
    for (auto it = _cfg->loggers.begin(); it != _cfg->loggers.end(); ++it) {
        // Source
        std::string name = it->first;
        Logger &pLogger = it->second;

        // Build Sink
        spdlog::sink_ptr ptr = nullptr;
        if (pLogger.appenders.size() == 1) {
            ptr = results[pLogger.appenders[0]];
        } else if (pLogger.appenders.size() > 1) {
            auto dist_sink = std::make_shared<dist_sink_st>();
            for (auto ait = pLogger.appenders.begin(); ait != pLogger.appenders.end(); ++ait) {
                dist_sink->add_sink(results[*ait]);
            }
            ptr = dist_sink;
        }

        // Read level
        spdlog::level::level_enum lvl = spdlog::level::info;
        switch (pLogger.level) {
        case Logger::Level::TRACE:
            lvl = spdlog::level::trace;
            break;
        case Logger::Level::DEBUG:
            lvl = spdlog::level::debug;
            break;
        case Logger::Level::INFO:
            lvl = spdlog::level::info;
            break;
        case Logger::Level::WARNING:
            lvl = spdlog::level::warn;
            break;
        case Logger::Level::ERROR:
            lvl = spdlog::level::err;
            break;
        case Logger::Level::CRITICAL:
            lvl = spdlog::level::critical;
            break;
        }

        // Create logger
        std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(name, ptr);
        logger->set_level(lvl);
        logger->set_pattern(pLogger.format);
        logger->flush_on(spdlog::level::err);
        spdlog::register_logger(logger);
    }

    // Check that root exists
    _root = spdlog::get("root");
    if (_root == nullptr) {
        throw std::runtime_error("Root logger not configured");
    }
}

// See ServiceImpl.h
void ServiceImpl::Stop() {}

// See ServiceImpl.h
std::shared_ptr<spdlog::logger> ServiceImpl::select(const std::string &name) noexcept {
    std::string tmp_name = name;
    std::shared_ptr<spdlog::logger> result = nullptr;
    do {
        result = spdlog::get(tmp_name);
        if (result) {
            return result;
        }

        std::size_t idx = tmp_name.find_last_of('.');
        if (idx == -1) {
            idx = 0;
        }

        tmp_name = tmp_name.substr(0, idx);
    } while (!tmp_name.empty());

    // Failed to find any
    return _root;
}

// See ServiceImpl.h
std::unique_ptr<spdlog::logger> ServiceImpl::create(const std::string &name,
                                                    const std::map<std::string, std::string> &mdc) noexcept {
    auto base = select(name);
    auto &sinks = base->sinks();

    std::stringstream ss;
    std::string pattern = base->pattern();

    // Compile pattern, replacing MDC variables by values
    auto end = pattern.end();
    std::string::iterator brace_end;
    for (auto it = pattern.begin(); it != end; it++) {
        if (*it != '%') {
            ss << *it;
            continue;
        }

        if (++it != end && *it != 'X') {
            ss << '%' << *it;
            continue;
        }

        if (++it != end && *it != '{') {
            ss << "%X" << *it;
            continue;
        }

        if ((brace_end = std::find(it, end, '}')) == end) {
            ss << "%X{" << *it;
            continue;
        }

        // extract key
        auto const &key = std::string(it + 1, brace_end);

        // replace key by its value
        auto pos = mdc.find(key);
        if (pos == mdc.end()) {
            ss << "%X{" << *it;
            continue;
        }

        ss << pos->first;
        it = brace_end;
    }

    // Done, create formatter
    auto result = std::unique_ptr<spdlog::logger>(new spdlog::logger(base->name(), sinks.begin(), sinks.end()));
    result->set_level(base->level());
    result->set_pattern(ss.str());
    result->flush_on(spdlog::level::err);
    return result;
}

// See ServiceImpl.h
void ServiceImpl::reopen_all() {
    // Collect all sinks
    std::deque<spdlog::sink_ptr> sinks;
    auto collect_sinks = [&sinks](std::shared_ptr<spdlog::logger> log) {
        SPDLOG_DEBUG("Collect sinks from: " + log->name());
        const std::vector<spdlog::sink_ptr> &log_sink = log->sinks();
        sinks.insert(sinks.end(), log_sink.begin(), log_sink.end());
    };
    spdlog::apply_all(collect_sinks);

    // Process collected sinks
    std::unordered_set<spdlog::sink_ptr> seen;
    while (!sinks.empty()) {
        spdlog::sink_ptr psink = sinks.front();
        sinks.pop_front();

        // Avoid loops
        if (seen.find(psink) != seen.end()) {
            continue;
        }
        seen.insert(psink);

        // Check for distribution sinks
        auto *dist_ptr = dynamic_cast<dist_sink_st *>(psink.get());
        if (dist_ptr != nullptr) {
            sinks.insert(sinks.end(), dist_ptr->sinks().begin(), dist_ptr->sinks().end());
            continue;
        }

        // Check for file sinks
        auto *file_sink = dynamic_cast<ext_rotate_file_sink_mt *>(psink.get());
        if (file_sink != nullptr) {
            file_sink->reopen();
        }
    }
}

} // namespace Logging
} // namespace Afina
