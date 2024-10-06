#pragma once

#include "jsystem/JKRHeap.hh"

#include <common/Types.hh>
#include <payload/Replace.hh>

class JKRTask {
public:
    typedef void (*Task)(void *userData);

    bool request(Task task, void *userData, void *r6);

    static JKRTask *REPLACED(Create)(s32 r3, s32 threadPriority, u32 stackSize, JKRHeap *heap);
    REPLACE static JKRTask *Create(s32 r3, s32 threadPriority, u32 stackSize, JKRHeap *heap);

private:
    u8 _00[0x98 - 0x00];
};
size_assert(JKRTask, 0x98);
