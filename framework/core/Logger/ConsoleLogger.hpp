#pragma once

#include "ILogger.hpp"
#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

class ConsoleLogger : public ILogger
{
public:
    void trace(const std::string& msg, uint16_t level = 0) override
    {
        log("TRACE", msg, level);
    }

    void debug(const std::string& msg, uint16_t level = 0) override
    {
        log("DEBUG", msg, level);
    }

    void info(const std::string& msg, uint16_t level = 0) override
    {
        log("INFO", msg, level);
    }

    void warn(const std::string& msg, uint16_t level = 0) override
    {
        log("WARN", msg, level);
    }

    void error(const std::string& msg, uint16_t level = 0) override
    {
        log("ERROR", msg, level);
    }

    void critical(const std::string& msg, uint16_t level = 0) override
    {
        log("CRITICAL", msg, level);
    }

private:
    std::mutex mutex_;

    static std::string currentTime()
    {
        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);

        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    void log(const std::string& tag, const std::string& msg, uint16_t level)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout
            << "[" << currentTime() << "] "
            << "[" << tag << "] "
            << "[Lvl:" << level << "] "
            << msg
            << '\n';
    }
};