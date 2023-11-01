#pragma once

#include "jsystem/JKRAramBlock.hh"
#include "jsystem/JKRHeap.hh"

class JKRAram {
public:
    static JKRAramBlock *MainRamToAram(const u8 *src, u32 dst, u32 size, s32 r6, u32 r7,
            JKRHeap *heap, s32 r9, u32 *r10);

private:
    u8 _00[0xa4 - 0x00];
};
size_assert(JKRAram, 0xa4);
