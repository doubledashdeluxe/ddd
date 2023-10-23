#include <common/Arena.hh>

#include <common/Align.hh>
extern "C" {
#include <dolphin/OSArena.h>
}

void *MEM1Arena::alloc(size_t size, s32 align) {
    if (align >= 0) {
        u32 lo = reinterpret_cast<u32>(OSGetArenaLo());
        lo = AlignUp<u32>(lo, align);
        void *ptr = reinterpret_cast<void *>(lo);
        lo += size;
        OSSetArenaLo(reinterpret_cast<void *>(lo));
        return ptr;
    } else {
        u32 hi = reinterpret_cast<u32>(OSGetArenaHi());
        hi = AlignDown<u32>(hi, -align);
        hi -= size;
        OSSetArenaHi(reinterpret_cast<void *>(hi));
        void *ptr = reinterpret_cast<void *>(hi);
        return ptr;
    }
}
