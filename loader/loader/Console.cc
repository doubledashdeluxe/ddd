#include "Console.hh"

#include "loader/Font.hh"
#include "loader/VI.hh"

void Console::Init(u8 bg, u8 fg) {
    s_bg = bg;
    s_fg = fg;
    s_cols = VI::GetXFBWidth() / Font::GetGlyphWidth() - 1;
    s_rows = VI::GetXFBHeight() / Font::GetGlyphHeight() / (!VI::IsProgressive() + 1) - 1;
    s_col = 0;
    s_row = 0;
}

void Console::Print(const char *s) {
    for (; *s; s++) {
        Print(*s);
    }
    VI::FlushXFB();
}

void Console::Print(u32 val) {
    for (u32 i = 0; i < 8; i++) {
        u32 digit = val >> ((7 - i) * 4) & 0xf;
        if (digit < 0xa) {
            Print(static_cast<char>(digit + '0'));
        } else {
            Print(static_cast<char>(digit - 0xa + 'A'));
        }
    }
}

void Console::Print(char c) {
    if (s_col >= s_cols) {
        return;
    }

    if (c == '\n') {
        s_col = 0;
        s_row++;
        return;
    }

    if (c == '\r') {
        s_col = 0;
        return;
    }

    while (s_row >= s_rows) {
        u16 xfbWidth = VI::GetXFBWidth();
        u8 glyphHeight = Font::GetGlyphHeight();
        for (u8 s_row = 0; s_row < s_rows; s_row++) {
            u16 y0 = s_row * glyphHeight + glyphHeight / 2;
            for (u16 y = 0; y < glyphHeight; y++) {
                for (u16 x = 0; x < xfbWidth; x++) {
                    u8 intensity = VI::ReadGrayscaleFromXFB(x, y0 + glyphHeight + y);
                    VI::WriteGrayscaleToXFB(x, y0 + y, intensity);
                }
            }
        }
        s_row--;
    }

    u8 glyphWidth = Font::GetGlyphWidth();
    u8 glyphHeight = Font::GetGlyphHeight();
    const u8 *glyph = Font::GetGlyph(c);
    u16 y0 = s_row * glyphHeight + glyphHeight / 2;
    for (u16 y = 0; y < glyphHeight; y++) {
        u16 x0 = s_col * glyphWidth + glyphWidth / 2;
        for (u16 x = 0; x < glyphWidth; x++) {
            u8 intensity = glyph[(y * glyphWidth + x) / 8] & (1 << (8 - (x % 8))) ? s_fg : s_bg;
            VI::WriteGrayscaleToXFB(x0 + x, y0 + y, intensity);
        }
    }

    s_col++;
}

u8 Console::s_bg;
u8 Console::s_fg;
u8 Console::s_cols;
u8 Console::s_rows;
u8 Console::s_col;
u8 Console::s_row;
