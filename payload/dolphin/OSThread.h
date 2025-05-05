#pragma once

#include "dolphin/OSContext.h"

#include <portable/Types.h>

enum {
    OS_THREAD_STATE_EXITED = 0,
    OS_THREAD_STATE_READY = 1 << 0,
    OS_THREAD_STATE_RUNNING = 1 << 1,
    OS_THREAD_STATE_SLEEPING = 1 << 2,
    OS_THREAD_STATE_MORIBUND = 1 << 3,
};

enum {
    OS_THREAD_DETACHED = 1 << 0,
};

typedef struct OSThreadQueue {
    struct OSThread *head;
    struct OSThread *tail;
} OSThreadQueue;
size_assert(OSThreadQueue, 0x8);

typedef struct OSThread {
    OSContext context;
    u16 state;
    u16 flags;
    u8 _2cc[0x2d8 - 0x2cc];
    void *val;
    u8 _2dc[0x2e8 - 0x2dc];
    OSThreadQueue joinQueue;
    u8 _2f0[0x2fc - 0x2f0];
    struct OSThread *nextActive;
    struct OSThread *prevActive;
    u8 _304[0x308 - 0x304];
    u32 *stackTop;
    u8 _30c[0x318 - 0x30c];
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
BOOL OSJoinThread(OSThread *thread, void **val);
void OSDetachThread(OSThread *thread);
s32 OSResumeThread(OSThread *thread);
s32 OSSuspendThread(OSThread *thread);
