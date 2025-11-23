#pragma once

extern "C" {
#include <dolphin/GX.h>
}

class Mario {
public:
    void draw(f32 t, GXColor color);

private:
    u8 _00[0x30 - 0x00];
};
size_assert(Mario, 0x30);
