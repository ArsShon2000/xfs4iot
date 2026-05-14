#pragma once
#include <string>
#include "LoggerLevels.hpp"

class ILogger {
public:
    virtual void trace(const std::string& msg, uint16_t level = 0) = 0;
    virtual void info(const std::string& msg, uint16_t level = 0) = 0;
    virtual void warn(const std::string& msg, uint16_t level = 0) = 0;
    virtual void error(const std::string& msg, uint16_t level = 0) = 0;
    virtual void debug(const std::string& msg, uint16_t level = 0) = 0;
    virtual void critical(const std::string& msg, uint16_t level = 0) = 0;
    virtual ~ILogger() = default;
};
