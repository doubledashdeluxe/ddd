#pragma once

#include <common/Types.hh>

class JKRHeap {
private:
    u8 _00[0x6c - 0x00];
};
static_assert(sizeof(JKRHeap) == 0x6c);
