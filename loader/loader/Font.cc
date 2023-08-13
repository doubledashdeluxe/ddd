#include "Font.hh"

extern "C" {
#include "loader/Font.h"
}

u8 Font::GetGlyphWidth() {
    return 8;
}

u8 Font::GetGlyphHeight() {
    return 16;
}

const u8 *Font::GetGlyph(char c) {
    u32 i = c;
    if (i < 128) {
        return font[i];
    }

    return font[' '];
}
