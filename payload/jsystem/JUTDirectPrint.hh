#pragma once

#include <portable/Types.hh>

class JUTDirectPrint {
public:
    void changeFrameBuffer(void *frameBuffer, u16 width, u16 height);

private:
    u8 _00[0x2c - 0x00];
};
size_assert(JUTDirectPrint, 0x2c);
