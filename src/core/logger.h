// =========================================================================
// File: src/core/Logger.h
// Leírás: A központi, Singleton logolási rendszer.
// =========================================================================
#pragma once

#include <Arduino.h>
#include <functional>
#include <cstdarg>

enum class LogLevel { NONE, ERROR, WARN, INFO, DEBUG, VERBOSE };

class Logger {
public:
    using LogCallback = std::function<void(LogLevel, const char*)>;

    static Logger& getInstance();

    void setLogLevel(LogLevel level);
    void onLog(const LogCallback& callback);

    void error(const char* format, ...);
    void warn(const char* format, ...);
    void info(const char* format, ...);
    void debug(const char* format, ...);
    void verbose(const char* format, ...);

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel _logLevel = LogLevel::INFO;
    LogCallback _callback = nullptr;

    void print(LogLevel level, const char* format, va_list args);
};

extern Logger* ha_log;