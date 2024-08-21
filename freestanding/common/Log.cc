#include <common/Log.hh>

#include <common/Console.hh>

extern "C" void Log(u32 level, const char *shortFormat, const char *longFormat, ...) {
    va_list vlist;
    va_start(vlist, longFormat);
    VLog(level, shortFormat, longFormat, vlist);
    va_end(vlist);
}

extern "C" void VLog(u32 level, const char *shortFormat, const char * /* longFormat */,
        va_list vlist) {
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
