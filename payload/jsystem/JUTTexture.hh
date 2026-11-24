#pragma once

#include <portable/Types.hh>

class JUTTexture {
public:
    JUTTexture(u32 width, u32 height, u32 format);

private:
    u8 _00[0x40 - 0x00];
};
size_assert(JUTTexture, 0x40);
