#include <common/Clock.hh>

void Clock::WaitMilliseconds(u32 milliseconds) {
    u32 duration = milliseconds * 60750;
    u32 start;
    asm volatile("mfspr %0, 268" : "=r"(start));
    for (u32 current = start; current - start < duration;) {
        asm volatile("mfspr %0, 268" : "=r"(current));
    }
}
