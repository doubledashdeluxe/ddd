#pragma once

#include <common/Types.hh>

class JKRAramHeap {
public:
    JKRAramHeap(u32 graphBuf, u32 graphBufSize);

private:
    u8 _00[0x44 - 0x00];
};
size_assert(JKRAramHeap, 0x44);
