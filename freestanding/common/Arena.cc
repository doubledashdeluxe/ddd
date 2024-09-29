#include <common/Arena.hh>

#include <common/Align.hh>

extern "C" {
#include <assert.h>
}

void *MEM1Arena::alloc(size_t size, s32 align) {
    assert(align <= 0);
    s_hi -= size;
    s_hi = AlignDown<u32>(s_hi, -align);
    void *ptr = reinterpret_cast<void *>(s_hi);
    return ptr;
}

u32 MEM1Arena::s_hi = 0x817fbb60;
