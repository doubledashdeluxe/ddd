#pragma once

#include "dolphin/OSContext.h"

#include <portable/Types.h>

typedef struct OSAlarm {
    u8 _00[0x28 - 0x00];
} OSAlarm;
size_assert(OSAlarm, 0x28);

typedef void (*OSAlarmHandler)(OSAlarm *alarm, OSContext *context);

void OSSetAlarm(OSAlarm *alarm, s64 tick, OSAlarmHandler handler);
void OSSetPeriodicAlarm(OSAlarm *alarm, s64 start, s64 period, OSAlarmHandler handler);
void OSCreateAlarm(OSAlarm *alarm);
void OSCancelAlarm(OSAlarm *alarm);
