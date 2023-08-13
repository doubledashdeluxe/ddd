#pragma once

#include <common/Types.h>

typedef struct OSThreadQueue OSThreadQueue;

void OSInitThreadQueue(OSThreadQueue *queue);
void OSSleepThread(OSThreadQueue *queue);
void OSWakeupThread(OSThreadQueue *queue);

s32 OSDisableScheduler(void);
s32 OSEnableScheduler(void);
void OSReschedule(void);
