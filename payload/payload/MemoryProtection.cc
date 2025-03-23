#include "MemoryProtection.hh"

#include "payload/DOLBinary.hh"
#include "payload/MI.hh"
#include "payload/PayloadBinary.hh"

void MemoryProtection::Finalize() {
    if (s_isFinalized) {
        return;
    }

    FinalizeBATs();
    FinalizeMI();
    s_isFinalized = true;
}

#ifdef __CWCC__
asm void MemoryProtection::FinalizeBATs() {
    // clang-format off

    nofralloc

    mflr r3
    bl EnterRealMode
    bl RunInRealMode

EnterRealMode:
    mflr r4
    clrlwi r4, r4, 1
    mtsrr0 r4
    mfmsr r4
    rlwinm r5, r4, 0, 28, 25
    mtsrr1 r5
    rfi

RunInRealMode:
    li r5, 0x0
    isync
    mtdbatu 3, r5
    mtibatu 3, r5
    isync

    lis r5, 0xc800
    ori r5, r5, 0x0fff
    lis r6, 0x0800
    ori r6, r6, 0x002a
    mtdbatl 3, r6
    mtdbatu 3, r5
    isync

    mflr r5
    bl Wipe

Wipe:
    mflr r6
    li r7, 0x0

Loop:
    cmpw r5, r6
    bge ExitRealMode
    stw r7, 0x0 (r5)
    dcbf 0x0, r5
    addi r5, r5, 0x4
    b Loop

ExitRealMode:
    mtsrr0 r3
    mtsrr1 r4
    isync
    rfi

    // clang-format on
}
#endif

void MemoryProtection::FinalizeMI() {
    // We would like to also protect the first page, but the IOS58 network module writes a few
    // statistics to it, blowing up exception handlers at the same time.
    MI::Protect(0, 0x00001000, 0x00360000, MI::R);
    MI::Protect(1, DOLBinary::RodataSectionStart(), DOLBinary::RodataSectionEnd(), MI::R);
    MI::Protect(2, DOLBinary::Sdata2SectionStart(), DOLBinary::Sbss2SectionEnd(), MI::R);
    MI::Protect(3, PayloadBinary::CtorsSectionStart(), PayloadBinary::RodataSectionEnd(), MI::R);
}

bool MemoryProtection::s_isFinalized = false;
