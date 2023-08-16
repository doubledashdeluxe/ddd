#pragma once

enum LogLevel {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
};

void Log(LogLevel level, const char *format, ...);

#define ERROR(...) Log(LOG_LEVEL_ERROR, __VA_ARGS__)
#define WARN(...) Log(LOG_LEVEL_WARN, __VA_ARGS__)
#define INFO(...) Log(LOG_LEVEL_INFO, __VA_ARGS__)
#define DEBUG(...) Log(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define TRACE(...) Log(LOG_LEVEL_TRACE, __VA_ARGS__)
