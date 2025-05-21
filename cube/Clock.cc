// clang-format off
//
// Resources:
// - https://wiibrew.org/wiki/Memory_map
//
// clang-format on

#include "Clock.hh"

extern "C" u32 busClock;

s64 Clock::TicksToMicroseconds(s64 ticks) {
    return (ticks * 1000000) / (busClock / 4);
}

s64 Clock::TicksToMilliseconds(s64 ticks) {
    return (ticks * 1000) / (busClock / 4);
}

s64 Clock::TicksToSeconds(s64 ticks) {
    return ticks / (busClock / 4);
}

s64 Clock::MicrosecondsToTicks(s64 microseconds) {
    return microseconds * (busClock / 4) / 1000000;
}

s64 Clock::MillisecondsToTicks(s64 milliseconds) {
    return milliseconds * (busClock / 4) / 1000;
}

s64 Clock::SecondsToTicks(s64 seconds) {
    return seconds * (busClock / 4);
}

void Clock::WaitMicroseconds(u32 microseconds) {
    WaitTicks(MicrosecondsToTicks(microseconds));
}

void Clock::WaitMilliseconds(u32 milliseconds) {
    WaitTicks(MillisecondsToTicks(milliseconds));
}

void Clock::WaitSeconds(u32 seconds) {
    WaitTicks(SecondsToTicks(seconds));
}
