#pragma once

#include "common/Types.hh"

#ifdef PAYLOAD
extern "C" {
#include <dolphin/OSAlarm.h>
}
#endif

class Clock {
public:
#ifdef PAYLOAD
    static void Init();
#endif

    static s64 TicksToMilliseconds(s64 ticks);
    static s64 TicksToSeconds(s64 ticks);
    static s64 MillisecondsToTicks(s64 milliseconds);
    static s64 SecondsToTicks(s64 seconds);

    static void WaitMilliseconds(u32 milliseconds);

private:
    Clock();

#ifdef PAYLOAD
    static bool ReadRTC(u32 &rtc);
    static bool ReadCounterBias(u32 &counterBias);

    static void HandleAlarm(OSAlarm *alarm, OSContext *context);
#endif
};
