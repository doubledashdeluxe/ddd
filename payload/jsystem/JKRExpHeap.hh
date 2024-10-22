#pragma once

#include "jsystem/JKRHeap.hh"

class JKRExpHeap : public JKRHeap {
public:
    static JKRExpHeap *Create(size_t size, JKRHeap *parent, bool errorFlag);
    static JKRExpHeap *Create(void *ptr, size_t size, JKRHeap *parent, bool errorFlag);

private:
    u8 _6c[0x88 - 0x6c];
};
size_assert(JKRExpHeap, 0x88);
