#include <cube/Clock.hh>

#include <cube/Platform.hh>

extern "C" u32 busClock;

void Clock::Init() {
    if (!Platform::IsGameCube()) {
        busClock = 243000000;
    }
}

#ifdef __CWCC__
s64 Clock::GetMonotonicTicks() {
    u32 prevHi, currLo, currHi;
    do {
        asm volatile("mftb %0, 269" : "=r"(prevHi));
        asm volatile("mftb %0, 268" : "=r"(currLo));
        asm volatile("mftb %0, 269" : "=r"(currHi));
    } while (currHi != prevHi);
    return static_cast<s64>(currHi) << 32 | currLo << 0;
}
#endif

void Clock::WaitTicks(s64 ticks) {
    s64 start = GetMonotonicTicks();
    s64 end = start + ticks;
    while (GetMonotonicTicks() < end) {}
}
