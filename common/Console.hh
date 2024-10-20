#pragma once

#include "common/VI.hh"

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

    void vprintf(Color bg, Color fg, const char *format, va_list vlist);

    static void Init(VI *vi);
    static void Init(Console *instance);
    static Console *Instance();

    bool m_isActive;

private:
    Console(VI *vi);

    void putchar(int c);
    void scroll();

    static void Putchar(int c, void *ctx);

    VI *m_vi;
    u8 m_cols;
    u8 m_rows;
    u8 m_col;
    u8 m_row;
    Color m_bg;
    Color m_fg;

    static Console *s_instance;
};
