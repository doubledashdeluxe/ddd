#include "Clock.hh"

extern "C" u32 busClock;

s64 Clock::TicksToMilliseconds(s64 ticks) {
    return (ticks * 1000) / (busClock / 4);
}

s64 Clock::TicksToSeconds(s64 ticks) {
    return ticks / (busClock / 4);
}

s64 Clock::MillisecondsToTicks(s64 milliseconds) {
    return milliseconds * (busClock / 4) / 1000;
}

s64 Clock::SecondsToTicks(s64 seconds) {
    return seconds * (busClock / 4);
}
