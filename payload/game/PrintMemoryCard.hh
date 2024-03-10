#pragma once

#include <common/Types.hh>

class PrintMemoryCard {
public:
    void ack(u32 soundID);

private:
    u8 _00[0x0c - 0x00];
    bool _c;
    bool _d;
    bool _e;
    u8 _0f[0x30 - 0x0f];
};
size_assert(PrintMemoryCard, 0x30);
