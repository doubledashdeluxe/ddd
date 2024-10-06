#include "JKRTask.hh"

JKRTask *JKRTask::Create(s32 r3, s32 threadPriority, u32 stackSize, JKRHeap *heap) {
    // Hack: this should be done in System::Init, but it's just way too long
    if (threadPriority == 18) {
        stackSize = 64 * 1024;
    }
    return REPLACED(Create)(r3, threadPriority, stackSize, heap);
}
