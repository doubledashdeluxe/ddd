#pragma once

#include <common/Types.hh>

class Font {
public:
    static u8 GetGlyphWidth();
    static u8 GetGlyphHeight();
    static const u8 *GetGlyph(char c);

private:
    Font();
};
