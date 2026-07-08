#include "logger.h"
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <mutex>

static std::mutex s_LogMutex;
static FILE *s_LogFile = nullptr;

void LogMessage(LogLevel level, const char *file, int line, const char *fmt, ...)
{
    std::lock_guard<std::mutex> lock(s_LogMutex);

    if (!s_LogFile)
    {
        s_LogFile = fopen("ebauche.log", "w");
    }

    const char *levelStr = "INFO";
    switch (level)
    {
    case LogLevel::WARN:
        levelStr = "WARN";
        break;
    case LogLevel::ERR:
        levelStr = "ERROR";
        break;
    case LogLevel::FATAL:
        levelStr = "FATAL";
        break;
    default:
        break;
    }

    time_t now = time(nullptr);
    struct tm tstruct;
    localtime_s(&tstruct, &now);
    char timebuf[80];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tstruct);

    char messagebuf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(messagebuf, sizeof(messagebuf), fmt, args);
    va_end(args);

    printf("[%s] [%s] %s:%d - %s\n", timebuf, levelStr, file, line, messagebuf);
    if (s_LogFile)
    {
        fprintf(s_LogFile, "[%s] [%s] %s:%d - %s\n", timebuf, levelStr, file, line, messagebuf);
        fflush(s_LogFile);
    }
}

void ShutdownLogger()
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    if (s_LogFile)
    {
        fclose(s_LogFile);
        s_LogFile = nullptr;
    }
}
