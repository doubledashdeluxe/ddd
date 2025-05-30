// clang-format off
//
// Resources:
// - https://github.com/dolphin-emu/dolphin/blob/2503a/Source/Core/Core/PowerPC/PowerPC.cpp
//
// clang-format on

#include "ECID.hh"

#ifdef __CWCC__
asm ECID ECID::Get() {
    // clang-format off

    nofralloc

    mfspr r4, 924
    stw r4, ECID.u (r3)
    mfspr r4, 925
    stw r4, ECID.m (r3)
    mfspr r4, 926
    stw r4, ECID.l (r3)
    blr

    // clang-format on
}
#endif
