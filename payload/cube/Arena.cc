#include <cube/Arena.hh>

extern "C" {
#include <dolphin/OSArena.h>
}
#include <portable/Align.hh>

extern "C" {
#include <assert.h>
}

void *MEM1Arena::alloc(size_t size, s32 align) {
    uintptr_t lo = reinterpret_cast<uintptr_t>(OSGetArenaLo());
    uintptr_t hi = reinterpret_cast<uintptr_t>(OSGetArenaHi());
    if (align >= 0) {
        lo = AlignUp<uintptr_t>(lo, align);
        void *ptr = reinterpret_cast<void *>(lo);
        lo += size;
        assert(lo <= hi);
        OSSetArenaLo(reinterpret_cast<void *>(lo));
        return ptr;
    } else {
        hi -= size;
        hi = AlignDown<uintptr_t>(hi, -align);
        assert(hi >= lo);
        OSSetArenaHi(reinterpret_cast<void *>(hi));
        void *ptr = reinterpret_cast<void *>(hi);
        return ptr;
    }
}
