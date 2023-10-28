#pragma once

#include "jsystem/JASTaskThread.hh"

#include <payload/Replace.hh>

class JASDvd {
public:
    static JASTaskThread *REPLACED(CreateThread)(s32 priority, s32 messageCount, u32 stackSize);
    REPLACE static JASTaskThread *CreateThread(s32 priority, s32 messageCount, u32 stackSize);

private:
    JASDvd();
};
static_assert(sizeof(JASDvd) == 0x1);
