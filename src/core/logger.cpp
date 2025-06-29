// =========================================================================
// File: src/core/Logger.cpp
// Leírás: A Logger osztály implementációja.
// =========================================================================
#include "Logger.h"

Logger* ha_log = &Logger::getInstance();

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(LogLevel level) {
    _logLevel = level;
}

void Logger::onLog(const LogCallback& callback) {
    _callback = callback;
}

void Logger::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print(LogLevel::ERROR, format, args);
    va_end(args);
}

void Logger::warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print(LogLevel::WARN, format, args);
    va_end(args);
}

void Logger::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print(LogLevel::INFO, format, args);
    va_end(args);
}

void Logger::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print(LogLevel::DEBUG, format, args);
    va_end(args);
}

void Logger::verbose(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print(LogLevel::VERBOSE, format, args);
    va_end(args);
}

void Logger::print(LogLevel level, const char* format, va_list args) {
    if (_logLevel >= level) {
        char buf[256];
        vsnprintf(buf, sizeof(buf), format, args);

        if (_callback) {
            _callback(level, buf);
        }
    }
}