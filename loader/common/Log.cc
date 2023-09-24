#include <common/Log.hh>

#include <common/Console.hh>

extern "C" void Log(LogLevel level, const char *format, ...) {
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
    va_list vlist;
    va_start(vlist, format);
    Console::VPrintf(format, vlist);
    va_end(vlist);
}
