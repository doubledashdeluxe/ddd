#pragma once

#include "common/Types.hh"

extern "C" {
struct OSAlarm;
struct OSContext;
}

class Clock {
public:
    static void Init();

    static s64 TicksToMilliseconds(s64 ticks);
    static s64 TicksToSeconds(s64 ticks);
    static s64 MillisecondsToTicks(s64 milliseconds);
    static s64 SecondsToTicks(s64 seconds);

    static s64 GetMonotonicTicks();

    static void WaitMilliseconds(u32 milliseconds);

private:
    Clock();

    static bool ReadRTC(u32 &rtc);
    static bool ReadCounterBias(u32 &counterBias);

    static void HandleAlarm(OSAlarm *alarm, OSContext *context);
};
