#include "NativeLogger.hh"

#include <cstdio>

void NativeLogger::vlog(u32 level, const char * /* shortFormat */, const char *longFormat,
        va_list vlist) {
    if (level == LOG_LEVEL_TRACE) {
        return;
    }

    vfprintf(stderr, longFormat, vlist);
}

NativeLogger::NativeLogger() = default;

NativeLogger::~NativeLogger() = default;

NativeLogger NativeLogger::s_instance;
