extern "C" {
#include "OSRtc.h"
}

#include <cube/Platform.hh>
#include <cube/SCFile.hh>
#include <portable/Bytes.hh>

extern "C" u8 *__OSLockSram();
extern "C" void __OSUnlockSram(BOOL write);

extern "C" u32 OSGetCounterBias() {
    if (Platform::IsGameCube()) {
        u8 *sram = __OSLockSram();
        u32 counterBias = Bytes::ReadBE<u32>(sram, 0x0c);
        __OSUnlockSram(false);
        return counterBias;
    }

    SCFile sc;
    u32 counterBias = 0;
    sc.get("IPL.CB", counterBias);
    return counterBias;
}
