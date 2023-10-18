#pragma once

#include <common/Types.hh>

class JKRHeap {
public:
    void *alloc(u32 size, s32 align);
    void free(void *ptr);
    s32 resize(void *ptr, u32 size);

    static JKRHeap *FindFromRoot(void *ptr);

private:
    u8 _00[0x6c - 0x00];
};
size_assert(JKRHeap, 0x6c);

void *operator new(size_t size, JKRHeap *heap, s32 alignment);
