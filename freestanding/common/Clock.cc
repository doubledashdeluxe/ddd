#include <common/Clock.hh>

extern "C" u32 busClock;

void Clock::Init() {
    busClock = 243000000;
}

#ifdef __CWCC__
void Clock::WaitMilliseconds(u32 milliseconds) {
    u32 duration = MillisecondsToTicks(milliseconds);
    u32 start;
    asm volatile("mfspr %0, 268" : "=r"(start));
    for (u32 current = start; current - start < duration;) {
        asm volatile("mfspr %0, 268" : "=r"(current));
    }
}
#endif
