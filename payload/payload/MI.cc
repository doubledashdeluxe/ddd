#include "MI.hh"

#include "payload/Lock.hh"

#include <common/Align.hh>
#include <common/Memory.hh>
extern "C" {
#include <dolphin/OSInterrupt.h>
}

struct MIChannel {
    u16 start;
    u16 end;
};

extern "C" volatile MIChannel mimarr[4];
extern "C" volatile u16 mimarrcontrol;

void MI::Protect(u32 channel, u32 start, u32 end, u32 flags) {
    Lock<NoInterrupts> lock;

    start = AlignUp(start, 4096);
    end = AlignDown(end, 4096);
    mimarr[channel].start = start >> 10;
    mimarr[channel].end = end >> 10;
    u16 control = mimarrcontrol;
    control &= ~((W | R) << (2 * channel));
    control |= flags << (2 * channel);
    mimarrcontrol = control;
    OSUnmaskInterrupts(1 << (31 - channel));
}

void MI::Protect(u32 channel, void *start, void *end, u32 flags) {
    Protect(channel, Memory::CachedToPhysical(start), Memory::CachedToPhysical(end), flags);
}
