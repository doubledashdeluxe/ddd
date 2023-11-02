#include <common/Arena.hh>

#include <common/Align.hh>
extern "C" {
#include <dolphin/OSArena.h>
}

void *MEM1Arena::alloc(size_t size, s32 align) {
    if (align >= 0) {
        uintptr_t lo = reinterpret_cast<uintptr_t>(OSGetArenaLo());
        lo = AlignUp<uintptr_t>(lo, align);
        void *ptr = reinterpret_cast<void *>(lo);
        lo += size;
        OSSetArenaLo(reinterpret_cast<void *>(lo));
        return ptr;
    } else {
        uintptr_t hi = reinterpret_cast<uintptr_t>(OSGetArenaHi());
        hi = AlignDown<uintptr_t>(hi, -align);
        hi -= size;
        OSSetArenaHi(reinterpret_cast<void *>(hi));
        void *ptr = reinterpret_cast<void *>(hi);
        return ptr;
    }
}
