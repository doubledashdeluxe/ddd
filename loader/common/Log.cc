#include <common/Log.hh>

#include <common/Console.hh>

extern "C" void Log(LogLevel level, const char *format, ...) {
    va_list vlist;
    va_start(vlist, format);
    VLog(level, format, vlist);
    va_end(vlist);
}

extern "C" void VLog(LogLevel level, const char *format, va_list vlist) {
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
    Console::VPrintf(format, vlist);
}
