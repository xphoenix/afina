#ifndef AFINA_LOGGING_CONFIG_H
#define AFINA_LOGGING_CONFIG_H

#include <map>
#include <string>
#include <vector>

namespace Afina {
namespace Logging {

// Describe outbound channel for log messages
class Appender {
public:
    enum Type { STDOUT, STDERR, FILE, DAILY, SIZED, SYSLOG };

    Appender() : rotate_at_hours(-1), rotate_at_mins(-1), rotate_at_size(0), option(0), facility(0){};

    /*
     * Appender type
     * Types: <ALL>
     */
    Appender::Type type;

    /*
     * Should console output be colorize
     * Types: STDOUT, STDERR
     */
    bool color;

    /*
     * File to write messages to
     * Types: FILE, DAILY, SIZED
     */
    std::string file;

    /*
     * Files suffix to be added during rotation
     * Types: DAILY, SIZED
     */
    std::string suffix;

    /*
     * Rotate file at given hours
     * Types: DAILY
     */
    int rotate_at_hours;

    /*
     * Rotate file at given minutes
     * Types: DAILY
     */
    int rotate_at_mins;

    /*
     * Size after which log should be rotated
     * TYpes: SIZED
     */
    uint64_t rotate_at_size;

    /*
     * NUmber of files to keep
     * Types: SIZED
     */
    int history_to_keep;

    /*
     * Ident logger
     * Types: SYSLOG
     */
    std::string ident;

    /*
     * Option for syslog, see man openlog, by default 0
     * Types: SYSLOG
     */
    int option = 0;

    /*
     * Facility for syslog, see man openlog, by default LOG_USER (0)
     */
    int facility = 0;
};

// Describe a logger
class Logger {
public:
    enum Level { CRITICAL, ERROR, WARNING, INFO, DEBUG, TRACE };

    /*
     * Minimum level that message must has to be written by the logger to the appender
     */
    Logger::Level level;

    /**
     * Message format to be used to print records in appenders
     */
    std::string format;

    /*
     * Name of append logger will writes to
     */
    std::vector<std::string> appenders;
};

/**
 * # Server logging layer
 */
class Config {
public:
    /*
     * Appenders by name
     */
    std::map<std::string, Appender> appenders;

    /**
     * Loggers by name
     */
    std::map<std::string, Logger> loggers;
};

} // namespace Logging
} // namespace Afina

#endif // AFINA_LOGGING_CONFIG_H
