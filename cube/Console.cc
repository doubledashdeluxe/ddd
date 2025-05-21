#include "Console.hh"

#include "cube/Arena.hh"

extern "C" {
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_VISIBILITY_STATIC
#define NANOPRINTF_IMPLEMENTATION
#include <nanoprintf.h>
}
#include <portable/Font.hh>

extern "C" {
#include <string.h>
}

const Console::Color Console::Color::Black = {0, 0, 0};
const Console::Color Console::Color::Blue = {0, 0, 255};
const Console::Color Console::Color::Green = {0, 255, 0};
const Console::Color Console::Color::Cyan = {0, 255, 255};
const Console::Color Console::Color::Red = {255, 0, 0};
const Console::Color Console::Color::Magenta = {255, 0, 255};
const Console::Color Console::Color::Yellow = {255, 255, 0};
const Console::Color Console::Color::White = {255, 255, 255};

void Console::vprintf(Color bg, Color fg, const char *format, va_list vlist) {
    if (!m_isActive) {
        return;
    }

    m_bg = bg;
    m_fg = fg;
    npf_vpprintf(Putchar, this, format, vlist);
}

void Console::Init(VI *vi) {
    s_instance = new (MEM1Arena::Instance(), -0x4) Console(vi);
}

void Console::Init(Console *instance) {
    s_instance = instance;
}

Console *Console::Instance() {
    return s_instance;
}

Console::Console(VI *vi)
    : m_isActive(true), m_vi(vi), m_cols(vi->getXFBWidth() / Font::GetGlyphWidth() - 1),
      m_rows(vi->getXFBHeight() / Font::GetGlyphHeight() - 1), m_col(0), m_row(0) {}

void Console::putchar(int c) {
    if (c == '\n') {
        m_col = 0;
        m_row++;
        return;
    }

    if (c == '\r') {
        m_col = 0;
        return;
    }

    if (m_col >= m_cols) {
        return;
    }

    scroll();

    u8 glyphWidth = Font::GetGlyphWidth();
    u8 glyphHeight = Font::GetGlyphHeight();
    const u8 *glyph = Font::GetGlyph(c);
    u16 y0 = (m_row % m_rows) * glyphHeight + glyphHeight / 2;
    for (u16 y = 0; y < glyphHeight; y++) {
        u16 x0 = m_col * glyphWidth + glyphWidth / 2;
        for (u16 x = 0; x < glyphWidth; x += 2) {
            u32 i0 = y * glyphWidth + x + 0;
            u32 i1 = y * glyphWidth + x + 1;
            Color color0 = glyph[i0 / 8] & (1 << (8 - (i0 % 8))) ? m_fg : m_bg;
            Color color1 = glyph[i1 / 8] & (1 << (8 - (i1 % 8))) ? m_fg : m_bg;
            Color colorm;
            colorm.r = (color0.r + color1.r) / 2;
            colorm.g = (color0.g + color1.g) / 2;
            colorm.b = (color0.b + color1.b) / 2;
            VI::Color color;
            color.y0 = 0.257f * color0.r + 0.504f * color0.g + 0.098f * color0.b + 16.0f;
            color.u = -0.148f * colorm.r - 0.291f * colorm.g + 0.439f * colorm.b + 128.0f;
            color.y1 = 0.257f * color1.r + 0.504f * color1.g + 0.098f * color1.b + 16.0f;
            color.v = 0.439f * colorm.r - 0.368f * colorm.g - 0.071f * colorm.b + 128.0f;
            m_vi->writeToXFB(x0 + x, y0 + y, color);
        }
    }

    m_col++;
}

void Console::scroll() {
    while (m_row >= m_rows) {
        u16 xfbWidth = m_vi->getXFBWidth();
        u8 glyphHeight = Font::GetGlyphHeight();
        for (u8 row = 0; row < m_rows; row++) {
            u16 y0 = row * glyphHeight + glyphHeight / 2;
            for (u16 y = 0; y < glyphHeight; y++) {
                for (u16 x = 0; x < xfbWidth; x++) {
                    VI::Color color = m_vi->readFromXFB(x, y0 + glyphHeight + y);
                    m_vi->writeToXFB(x, y0 + y, color);
                }
            }
        }
        m_row--;
    }
}

void Console::Putchar(int c, void *ctx) {
    static_cast<Console *>(ctx)->putchar(c);
}

Console *Console::s_instance = nullptr;
