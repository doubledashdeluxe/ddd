#pragma once

#include <common/Types.hh>

class JKRHeap {
public:
    void becomeCurrentHeap();
    void destroy();
    void *alloc(u32 size, s32 align);
    void free(void *ptr);
    void freeAll();
    s32 resize(void *ptr, u32 size);
    u32 getFreeSize();

    void *getStartAddr() const;
    void *getEndAddr() const;

    static JKRHeap *GetSystemHeap();
    static JKRHeap *GetCurrentHeap();
    static JKRHeap *GetRootHeap();
    static JKRHeap *FindFromRoot(void *ptr);

private:
    u8 _00[0x30 - 0x00];
    void *m_startAddr;
    void *m_endAddr;
    u8 _38[0x6c - 0x38];

    static JKRHeap *s_systemHeap;
    static JKRHeap *s_currentHeap;
    static JKRHeap *s_rootHeap;
};
size_assert(JKRHeap, 0x6c);

void *operator new(size_t size, JKRHeap *heap, s32 alignment);
void *operator new[](size_t size, JKRHeap *heap, s32 alignment);
