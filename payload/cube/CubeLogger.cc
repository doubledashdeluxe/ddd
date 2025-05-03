#include <cube/CubeLogger.hh>

#include <cube/Console.hh>
#include <payload/Lock.hh>
#include <payload/LogFile.hh>

extern "C" {
#include <stdio.h>
}

void CubeLogger::vlog(u32 level, const char *shortFormat, const char *longFormat, va_list vlist) {
    if (level == LOG_LEVEL_TRACE) {
        return;
    }

    va_list copy;
    va_copy(copy, vlist);
    vprintf(longFormat, copy);

    va_copy(copy, vlist);
    LogFile::VPrintf(longFormat, copy);

    Lock<NoInterrupts> lock;
    Console::Color bg = Console::Color::Black;
    Console::Color fg;
    switch (level) {
    case LOG_LEVEL_ERROR:
        fg = Console::Color::Red;
        break;
    case LOG_LEVEL_WARN:
        fg = Console::Color::Yellow;
        break;
    case LOG_LEVEL_INFO:
        fg = Console::Color::White;
        break;
    default:
        return;
    }
    Console::Instance()->vprintf(bg, fg, shortFormat, vlist);
}
