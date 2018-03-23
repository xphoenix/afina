#ifndef AFINA_LOGGER_H
#define AFINA_LOGGER_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <thread>

#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <thread>
#include <cstring>
#include <iomanip>

#define MSK (+3)

namespace Afina {

class Logger {
public:
    static Logger& Instance() {
        static Logger logger;
        return logger;
    }

    template<class T>
    std::string parse_arguments(T t) {
        std::stringstream ss;
        ss << t;
        return ss.str();
    }

    template <class T, class ...Args>
    std::string parse_arguments(T t, Args... args) {
        std::stringstream ss;
        ss << t;
        ss << " " << parse_arguments(args...);
        return ss.str();
    };

    template <class T, class ...Args>
    void write(T t, Args... args) {

        std::string str = parse_arguments(t, args...);

        std::lock_guard<std::mutex> lock(write_lock);

        std::chrono::time_point<std::chrono::system_clock> now =
            std::chrono::system_clock::now();

        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        time_t raw_time;
        struct tm *ptm;

        time(&raw_time);

        ptm = gmtime(&raw_time);

        std::stringstream ss;

        ss << "[ " << std::this_thread::get_id() << " ";

        ss << std::setfill('0') << std::setw(2) << (ptm->tm_year + 1900) << "-";
        ss << std::setfill('0') << std::setw(2) << ptm->tm_mon + 1 << "-";
        ss << std::setfill('0') << std::setw(2) << ptm->tm_mday << " ";
        ss << std::setfill('0') << std::setw(2) << (ptm->tm_hour + MSK) % 24 << ":";
        ss << std::setfill('0') << std::setw(2) << ptm->tm_min << ":";
        ss << std::setfill('0') << std::setw(2) << ptm->tm_sec;
        ss << "." << std::setfill('0') << std::setw(3) << millis % 1000;

        auto it = who_is_who.find(std::this_thread::get_id());
        if (it != who_is_who.end()) {
            ss << " " << it->second;
        }

        ss << " ] ";

        ss << str;

        std::cout << ss.str() << std::endl;
    }

    void i_am(std::string name) {
        std::lock_guard<std::mutex> lock(write_lock);
        who_is_who[std::this_thread::get_id()] = name;
    }


private:
    Logger() = default;
    ~Logger() {}

    Logger(Logger const&) = delete;
    Logger& operator= (Logger const&) = delete;

    std::mutex write_lock;
    std::unordered_map<std::thread::id, std::string> who_is_who;

};

};

#endif




