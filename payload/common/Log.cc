#include <common/Log.hh>

#include <common/Console.hh>
#include <payload/Lock.hh>
#include <payload/LogFile.hh>

extern "C" {
#include <stdio.h>
}

extern "C" void Log(LogLevel level, const char *format, ...) {
    if (level == LOG_LEVEL_TRACE) {
        return;
    }

    va_list vlist;
    va_start(vlist, format);
    vprintf(format, vlist);
    va_end(vlist);

    va_start(vlist, format);
    LogFile::VPrintf(format, vlist);
    va_end(vlist);

    Lock<NoInterrupts> lock;
    switch (level) {
    case LOG_LEVEL_ERROR:
        Console::s_fg = Console::Color::Red;
        break;
    case LOG_LEVEL_WARN:
        Console::s_fg = Console::Color::Yellow;
        break;
    case LOG_LEVEL_INFO:
        Console::s_fg = Console::Color::White;
        break;
    default:
        return;
    }
    va_start(vlist, format);
    Console::VPrintf(format, vlist);
    va_end(vlist);
}
