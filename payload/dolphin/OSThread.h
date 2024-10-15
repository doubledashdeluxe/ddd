#pragma once

#include "dolphin/OSContext.h"

typedef struct OSThreadQueue {
    u8 _0[0x8 - 0x0];
} OSThreadQueue;
size_assert(OSThreadQueue, 0x8);

typedef struct OSThread {
    OSContext context;
    u8 _2c8[0x318 - 0x2c8];
} OSThread;
size_assert(OSThread, 0x318);

void OSInitThreadQueue(OSThreadQueue *queue);
OSThread *OSGetCurrentThread(void);
BOOL OSIsThreadTerminated(OSThread *thread);
void OSSleepThread(OSThreadQueue *queue);
void OSWakeupThread(OSThreadQueue *queue);

s32 OSDisableScheduler(void);
s32 OSEnableScheduler(void);
void OSReschedule(void);

BOOL OSCreateThread(OSThread *thread, void *(*func)(void *), void *param, void *stack,
        u32 stackSize, s32 priority, u16 attr);
void OSDetachThread(OSThread *thread);
s32 OSResumeThread(OSThread *thread);
s32 OSSuspendThread(OSThread *thread);
