#pragma once

#include <portable/Types.hh>

class JKRThread {
public:
    JKRThread(u32 stackSize, s32 messageCount, s32 priority);
    virtual ~JKRThread();

protected:
    virtual void *run();

private:
    u8 _04[0x7c - 0x04];
};
size_assert(JKRThread, 0x7c);
