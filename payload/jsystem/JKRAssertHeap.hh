#pragma once

#include "jsystem/JKRHeap.hh"

class JKRAssertHeap : public JKRHeap {
public:
    static JKRAssertHeap *Create(JKRHeap *parent);

private:
    u8 _6c[0x70 - 0x6c];
};
size_assert(JKRAssertHeap, 0x70);
