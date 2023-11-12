#pragma once

#include <jsystem/JKRHeap.hh>

class SysDebug {
public:
    void setHeapGroup(const char *group, JKRHeap *heap);
    void setDefaultHeapGroup(JKRHeap *heap);
    void createHeapInfo(JKRHeap *heap, const char *name);

    static SysDebug *GetManager();

private:
    u8 _000[0x140 - 0x000];

    static SysDebug *s_manager;
};
size_assert(SysDebug, 0x140);
