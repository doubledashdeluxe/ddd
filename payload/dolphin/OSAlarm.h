#pragma once

#include "dolphin/OSContext.h"

typedef struct OSAlarm {
    u8 _00[0x28 - 0x00];
    void *userData;
} OSAlarm;
static_assert(sizeof(OSAlarm) == 0x2c);

typedef void (*OSAlarmHandler)(OSAlarm *alarm, OSContext *context);

void OSSetAlarm(OSAlarm *alarm, s64 tick, OSAlarmHandler handler);
void OSCreateAlarm(OSAlarm *alarm);
