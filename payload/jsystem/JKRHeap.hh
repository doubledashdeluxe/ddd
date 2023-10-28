#pragma once

#include <common/Types.hh>

class JKRHeap {
public:
    void *alloc(u32 size, s32 align);
    void free(void *ptr);

private:
    u8 _00[0x6c - 0x00];
};
static_assert(sizeof(JKRHeap) == 0x6c);
