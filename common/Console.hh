#pragma once

#include <common/Types.hh>

extern "C" {
#include <stdarg.h>
}

class Console {
public:
    struct Color {
        u8 r;
        u8 g;
        u8 b;

        static const Color Black;
        static const Color Blue;
        static const Color Green;
        static const Color Cyan;
        static const Color Red;
        static const Color Magenta;
        static const Color Yellow;
        static const Color White;
    };

    static void Init();
    static void Deinit();
    static void VPrintf(const char *format, va_list vlist);

    static Color s_bg;
    static Color s_fg;

private:
    Console();

    static void Putchar(int c, void *ctx);

    static bool s_isInit;
    static u8 s_cols;
    static u8 s_rows;
    static u8 s_col;
    static u8 s_row;
};
