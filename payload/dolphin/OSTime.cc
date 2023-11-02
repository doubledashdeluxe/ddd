extern "C" {
#include "OSTime.h"

#include "dolphin/EXIBios.h"
}

#include <payload/Lock.hh>

extern "C" s64 systemTimeOffset;

#ifdef __CWCC__
extern "C" void OSSetTime(s64 time) {
    Lock<NoInterrupts> lock;

    systemTimeOffset += OSGetTime() - time;
    u32 hi = time >> 32;
    u32 lo = time;
    asm("mttbl %0; mttbu %1; mttbl %2" : : "r"(0), "r"(hi), "r"(lo));
    EXIProbeReset();
}
#endif
