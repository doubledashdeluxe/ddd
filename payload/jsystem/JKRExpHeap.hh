#pragma once

#include "jsystem/JKRHeap.hh"

class JKRExpHeap : public JKRHeap {
public:
    class MemBlock {
    public:
        u32 getSize() const;
        const MemBlock *getPrev() const;
        const MemBlock *getNext() const;

    private:
        u8 _00[0x04 - 0x00];
        u32 m_size;
        MemBlock *m_prev;
        MemBlock *m_next;
    };
    size_assert(MemBlock, 0x10);

    const MemBlock *getFreeListHead() const;

    static JKRExpHeap *Create(size_t size, JKRHeap *parent, bool errorFlag);
    static JKRExpHeap *Create(void *ptr, size_t size, JKRHeap *parent, bool errorFlag);
    static u32 GetHeapType();

private:
    u8 _6c[0x78 - 0x6c];
    MemBlock *m_freeListHead;
    u8 _7c[0x88 - 0x7c];
};
size_assert(JKRExpHeap, 0x88);
