#pragma once

#include "portable/Types.h"

#include <stdarg.h>

enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
};

void Log(u32 level, const char *shortFormat, const char *longFormat, ...);
void VLog(u32 level, const char *shortFormat, const char *longFormat, va_list vlist);

#define STRINGIFY2(s) #s
#define STRINGIFY(s) STRINGIFY2(s)

#define PREFIX "[" __FILE__ ":" STRINGIFY(__LINE__) "] "
#define SUFFIX "%s\n"

#define LOG(level, format, ...) Log(level, format SUFFIX, PREFIX format SUFFIX, __VA_ARGS__)
#define VLOG(level, format, vlist) VLog(level, format, format, vlist)

#define ERROR(...) LOG(LOG_LEVEL_ERROR, __VA_ARGS__, "")
#define WARN(...) LOG(LOG_LEVEL_WARN, __VA_ARGS__, "")
#define INFO(...) LOG(LOG_LEVEL_INFO, __VA_ARGS__, "")
#define DEBUG(...) LOG(LOG_LEVEL_DEBUG, __VA_ARGS__, "")
#define TRACE(...) LOG(LOG_LEVEL_TRACE, __VA_ARGS__, "")

#define VERROR(format, vlist) VLOG(LOG_LEVEL_ERROR, format, vlist)
#define VWARN(format, vlist) VLOG(LOG_LEVEL_WARN, format, vlist)
#define VINFO(format, vlist) VLOG(LOG_LEVEL_INFO, format, vlist)
#define VDEBUG(format, vlist) VLOG(LOG_LEVEL_DEBUG, format, vlist)
#define VTRACE(format, vlist) VLOG(LOG_LEVEL_TRACE, format, vlist)
