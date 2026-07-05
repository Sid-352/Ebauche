#pragma once

#include <string>

enum class LogLevel
{
    INFO,
    WARN,
    ERR,
    FATAL
};

void LogMessage(LogLevel level, const char *file, int line, const char *fmt, ...);

#define LOG_INFO(...) LogMessage(LogLevel::INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) LogMessage(LogLevel::WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) LogMessage(LogLevel::ERR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) LogMessage(LogLevel::FATAL, __FILE__, __LINE__, __VA_ARGS__)
