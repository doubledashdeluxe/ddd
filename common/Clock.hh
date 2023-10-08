#pragma once

#include "common/Types.hh"

#ifdef PAYLOAD
extern "C" {
#include <dolphin/OSAlarm.h>
}
#endif

class Clock {
public:
    static s64 TicksToMilliseconds(s64 ticks);
    static s64 TicksToSeconds(s64 ticks);
    static s64 MillisecondsToTicks(s64 milliseconds);
    static s64 SecondsToTicks(s64 seconds);

    static void WaitMilliseconds(u32 milliseconds);

private:
    Clock();

#ifdef PAYLOAD
    static void HandleAlarm(OSAlarm *alarm, OSContext *context);
#endif
};
