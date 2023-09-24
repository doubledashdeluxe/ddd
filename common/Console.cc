#include "Console.hh"

#include "common/Font.hh"
#include "common/VI.hh"

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

const Console::Color Console::Color::Black = {0, 0, 0};
const Console::Color Console::Color::Blue = {0, 0, 255};
const Console::Color Console::Color::Green = {0, 255, 0};
const Console::Color Console::Color::Cyan = {0, 255, 255};
const Console::Color Console::Color::Red = {255, 0, 0};
const Console::Color Console::Color::Magenta = {255, 0, 255};
const Console::Color Console::Color::Yellow = {255, 255, 0};
const Console::Color Console::Color::White = {255, 255, 255};

void Console::Init() {
    s_cols = VI::GetXFBWidth() / Font::GetGlyphWidth() - 1;
    s_rows = VI::GetXFBHeight() / Font::GetGlyphHeight() / (!VI::IsProgressive() + 1) - 1;
    s_col = 0;
    s_row = 0;
    s_isInit = true;
}

void Console::Deinit() {
    s_isInit = false;
}

void Console::VPrintf(const char *format, va_list vlist) {
    if (!s_isInit) {
        return;
    }

    npf_vpprintf(Putchar, nullptr, format, vlist);
    VI::FlushXFB();
}

void Console::Putchar(int c, void * /* ctx */) {
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
                    VI::Color color = VI::ReadFromXFB(x, y0 + glyphHeight + y);
                    VI::WriteToXFB(x, y0 + y, color);
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
        for (u16 x = 0; x < glyphWidth; x += 2) {
            u32 i0 = y * glyphWidth + x + 0;
            u32 i1 = y * glyphWidth + x + 1;
            Color color0 = glyph[i0 / 8] & (1 << (8 - (i0 % 8))) ? s_fg : s_bg;
            Color color1 = glyph[i1 / 8] & (1 << (8 - (i1 % 8))) ? s_fg : s_bg;
            Color colorm;
            colorm.r = (color0.r + color1.r) / 2;
            colorm.g = (color0.g + color1.g) / 2;
            colorm.b = (color0.b + color1.b) / 2;
            VI::Color color;
            color.y0 = 0.257f * color0.r + 0.504f * color0.g + 0.098f * color0.b + 16.0f;
            color.u = -0.148f * colorm.r - 0.291f * colorm.g + 0.439f * colorm.b + 128.0f;
            color.y1 = 0.257f * color1.r + 0.504f * color1.g + 0.098f * color1.b + 16.0f;
            color.v = 0.439f * colorm.r - 0.368f * colorm.g - 0.071f * colorm.b + 128.0f;
            VI::WriteToXFB(x0 + x, y0 + y, color);
        }
    }

    s_col++;
}

Console::Color Console::s_bg = {0, 0, 0};
Console::Color Console::s_fg = {255, 255, 255};
bool Console::s_isInit = false;
u8 Console::s_cols;
u8 Console::s_rows;
u8 Console::s_col;
u8 Console::s_row;
