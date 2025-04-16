extern "C" {
#include "OSRtc.h"
}

#include <portable/Bytes.hh>

extern "C" u8 *__OSLockSram();
extern "C" void __OSUnlockSram(BOOL write);

extern "C" u32 OSGetCounterBias() {
    u8 *sram = __OSLockSram();
    u32 counterBias = Bytes::ReadBE<u32>(sram, 0x0c);
    __OSUnlockSram(false);
    return counterBias;
}
