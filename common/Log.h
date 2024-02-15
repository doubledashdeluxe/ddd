#pragma once

#include <stdarg.h>

enum LogLevel {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
};

void Log(LogLevel level, const char *format, ...);
void VLog(LogLevel level, const char *format, va_list vlist);

#define LOG(level, format, ...) Log(level, format "%s\n", __VA_ARGS__)

#define ERROR(...) LOG(LOG_LEVEL_ERROR, __VA_ARGS__, "")
#define WARN(...) LOG(LOG_LEVEL_WARN, __VA_ARGS__, "")
#define INFO(...) LOG(LOG_LEVEL_INFO, __VA_ARGS__, "")
#define DEBUG(...) LOG(LOG_LEVEL_DEBUG, __VA_ARGS__, "")
#define TRACE(...) LOG(LOG_LEVEL_TRACE, __VA_ARGS__, "")

#define VERROR(format, vlist) VLog(LOG_LEVEL_ERROR, format, vlist)
#define VWARN(format, vlist) VLog(LOG_LEVEL_WARN, format, vlist)
#define VINFO(format, vlist) VLog(LOG_LEVEL_INFO, format, vlist)
#define VDEBUG(format, vlist) VLog(LOG_LEVEL_DEBUG, format, vlist)
#define VTRACE(format, vlist) VLog(LOG_LEVEL_TRACE, format, vlist)
