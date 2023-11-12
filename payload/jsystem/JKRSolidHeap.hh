#pragma once

#include "jsystem/JKRHeap.hh"

class JKRSolidHeap : public JKRHeap {
public:
    static JKRSolidHeap *Create(size_t size, JKRHeap *parent, bool errorFlag);

private:
    u8 _6c[0x7c - 0x6c];
};
size_assert(JKRSolidHeap, 0x7c);
