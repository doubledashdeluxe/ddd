#include <common/Log.hh>

#include <loader/loader/Console.hh>
#include <loader/loader/VI.hh>
extern "C" {
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_VISIBILITY_STATIC
#define NANOPRINTF_IMPLEMENTATION
#include <vendor/nanoprintf.h>
}

static void Putchar(int c, void * /* ctx */) {
    Console::Putchar(c);
}

extern "C" void Log(LogLevel level, const char *format, ...) {
    switch (level) {
    case LOG_LEVEL_ERROR:
        Console::s_fg = Console::Color::Red;
        break;
    case LOG_LEVEL_WARN:
        Console::s_fg = Console::Color::Yellow;
        break;
    default:
        Console::s_fg = Console::Color::White;
        break;
    }
    va_list vlist;
    va_start(vlist, format);
    npf_vpprintf(Putchar, nullptr, format, vlist);
    va_end(vlist);
    VI::FlushXFB();
}
