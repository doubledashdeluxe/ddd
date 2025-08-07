#include "payload/Patcher.hh"
#include "payload/Payload.hh"
#include "payload/PayloadBinary.hh"

#include <cube/Arena.hh>
#include <cube/Context.hh>
#include <cube/Platform.hh>
extern "C" {
#include <dolphin/DB.h>
#include <dolphin/OS.h>

#include <stdlib.h>
}

#ifdef __CWCC__
#pragma section RX "first"
#endif

extern "C" void __init_registers();
extern "C" void __init_hardware();
extern "C" void __init_data();
extern "C" void __init_user();
int main();

extern "C" void RunPayload(Context *context) {
    void (**ctorsStart)() = static_cast<void (**)()>(PayloadBinary::CtorsSectionStart());
    void (**ctorsEnd)() = static_cast<void (**)()>(PayloadBinary::CtorsSectionEnd());
    for (void (**ctor)() = ctorsStart; ctor < ctorsEnd; ctor++) {
        (*ctor)();
    }

    Patcher::Run();

    if (!Platform::IsGameCube()) {
        MEM2Arena::Init(context->mem2ArenaLo, context->mem2ArenaHi);
    }
    VI::Init(context->vi);
    Console::Init(context->console);

    DBInit();
    OSInit();
    __init_user();

    MEM1Arena::Init();
    Payload::Run(context);
}

#ifdef __CWCC__
extern "C" __declspec(section "first") asm void Start(Context *context) {
    // clang-format off

    nofralloc

    lis r4, 0x40 / 0x20
    mtctr r4
    lis r4, 0x8040

Flush:
    dcbf 0x0, r4
    addi r4, r4, 0x20
    bdnz Flush
    sync

    bl EnterRealMode
    bl RunInRealMode
    b Continue

EnterRealMode:
    mflr r4
    clrlwi r4, r4, 1
    mtsrr0 r4
    li r4, 0x2000
    mtsrr1 r4
    rfi

RunInRealMode:
    li r4, 0x0
    isync
    mtdbatu 0, r4
    mtdbatu 1, r4
    mtdbatu 2, r4
    mtdbatu 3, r4
    mtibatu 0, r4
    mtibatu 1, r4
    mtibatu 2, r4
    mtibatu 3, r4
    isync

    lis r4, 0x8000
    ori r4, r4, 0x007f
    lis r5, 0x0040
    ori r5, r5, 0x0002
    mtdbatl 0, r5
    mtdbatu 0, r4
    isync

    lis r4, 0x8080
    ori r4, r4, 0x00ff
    lis r5, 0x0080
    ori r5, r5, 0x0002
    mtdbatl 1, r5
    mtdbatu 1, r4
    isync

    lis r4, 0x8100
    ori r4, r4, 0x00ff
    lis r5, 0x0100
    ori r5, r5, 0x0002
    mtdbatl 2, r5
    mtdbatu 2, r4
    isync

    lis r4, 0xc000
    ori r4, r4, 0x1fff
    lis r5, 0x0000
    ori r5, r5, 0x002a
    mtdbatl 3, r5
    mtdbatu 3, r4
    isync

    lis r4, 0x8000
    ori r4, r4, 0x003f
    lis r5, 0x0000
    ori r5, r5, 0x0001
    mtibatl 0, r5
    mtibatu 0, r4
    isync

    lis r4, 0x8020
    ori r4, r4, 0x001f
    lis r5, 0x0020
    ori r5, r5, 0x0001
    mtibatl 1, r5
    mtibatu 1, r4
    isync

    lis r4, 0x8030
    ori r4, r4, 0x0007
    lis r5, 0x0030
    ori r5, r5, 0x0001
    mtibatl 2, r5
    mtibatu 2, r4
    isync

    lis r4, 0x8034
    ori r4, r4, 0x0003
    lis r5, 0x0034
    ori r5, r5, 0x0001
    mtibatl 3, r5
    mtibatu 3, r4
    isync

    mflr r5
    oris r4, r5, 0x8000
    bl Wipe

Wipe:
    mflr r6
    li r7, 0x0

Loop:
    addi r5, r5, 0x4
    cmpw r5, r6
    bge ExitRealMode
    stw r7, 0x0 (r5)
    dcbf 0x0, r5
    b Loop

ExitRealMode:
    mtsrr0 r4
    li r4, 0x2030
    mtsrr1 r4
    isync
    rfi

Continue:
    lis r4, 0x40 / 0x20
    mtctr r4
    lis r4, 0x8000

Invalidate:
    dcbi 0x0, r4
    addi r4, r4, 0x20
    bdnz Invalidate

    mtctr r3
    bl __init_registers
    mfctr r30

    li r0, -0x1
    stwu r1, -0x8 (r1)
    stw r0, 0x4 (r1)
    stw r0, 0x0 (r1)

    bl __init_hardware
    bl __init_data

    mr r3, r30
    bl RunPayload

    bl main
    bl exit

    // clang-format on
}
#endif
