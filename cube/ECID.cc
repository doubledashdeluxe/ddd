// clang-format off
//
// Resources:
// - https://github.com/dolphin-emu/dolphin/blob/2503a/Source/Core/Core/PowerPC/PowerPC.cpp
//
// clang-format on

#include "ECID.hh"

#ifdef __CWCC__
asm Array<u32, 4> ECID::Get() {
    // clang-format off

    nofralloc

    mfspr r4, 924
    stw r4, 0x0 (r3)
    mfspr r4, 925
    stw r4, 0x4 (r3)
    mfspr r4, 926
    stw r4, 0x8 (r3)
    mfspr r4, 927
    stw r4, 0xc (r3)
    blr

    // clang-format on
}
#endif
