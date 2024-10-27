#pragma once

#include "jsystem/JKRAramBlock.hh"

class JKRAramHeap {
public:
    class AllocMode {
    public:
        enum {
            Head = 0,
            Tail = 1,
        };

    private:
        AllocMode();
    };

    JKRAramHeap(u32 graphBuf, u32 graphBufSize);

    JKRAramBlock *alloc(u32 size, u32 mode);

private:
    u8 _00[0x44 - 0x00];
};
size_assert(JKRAramHeap, 0x44);
