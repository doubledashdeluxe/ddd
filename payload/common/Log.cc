#include <common/Log.hh>

extern "C" {
#include <stdio.h>
}

extern "C" void Log(LogLevel /* level */, const char *format, ...) {
    va_list vlist;
    va_start(vlist, format);
    vprintf(format, vlist);
    va_end(vlist);
}
