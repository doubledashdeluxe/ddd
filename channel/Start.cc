// clang-format off
//
// Resources:
// - https://mariokartwii.com/showthread.php?tid=1963
//
// clang-format on

#include "channel/Channel.hh"
#include "channel/ChannelBinary.hh"

#include <common/Arena.hh>
#include <common/Platform.hh>
extern "C" {
#include <common/StackCanary.h>

#include <string.h>
}

#ifdef __CWCC__
#pragma section RX "first"
#endif

extern "C" u32 stackTop;

extern "C" void RunChannel() {
    void *bssStart = ChannelBinary::BssSectionStart();
    size_t bssSize = ChannelBinary::BssSectionSize();
    memset(bssStart, 0, bssSize);

    void (**ctorsStart)() = reinterpret_cast<void (**)()>(ChannelBinary::CtorsSectionStart());
    void (**ctorsEnd)() = reinterpret_cast<void (**)()>(ChannelBinary::CtorsSectionEnd());
    for (void (**ctor)() = ctorsStart; ctor < ctorsEnd; ctor++) {
        (*ctor)();
    }

    MEM1Arena::Init();
    if (!Platform::IsGameCube()) {
        MEM2Arena::Init(0x91000000, 0x93400000);
    }
    Context *context = new (MEM1Arena::Instance(), -0x4) Context;
    Channel::PayloadEntryFunc payloadEntry = Channel::Run(context);
    if (!Platform::IsGameCube()) {
        context->mem2ArenaLo = reinterpret_cast<uintptr_t>(MEM2Arena::Instance()->alloc(0x0, 0x4));
        context->mem2ArenaHi = reinterpret_cast<uintptr_t>(MEM2Arena::Instance()->alloc(0x0, -0x4));
    }
    context->vi = VI::Instance();
    context->console = Console::Instance();
    if (payloadEntry) {
        (*payloadEntry)(context);
    }

    while (true) {} // We should never get there
}

#ifdef __CWCC__
#pragma push
#pragma processor 7400
extern "C" __declspec(section "first") asm void Start() {
    // clang-format off

    nofralloc

    bl InitCPU

    // Initialize the stack pointer
    lis r1, stackTop@h
    ori r1, r1, stackTop@l

    // Initialize the stack canary
    lis r3, StackCanary_Init@h
    ori r3, r3, StackCanary_Init@l
    mtctr r3
    bctrl

    // Jump to C++ code
    lis r3, RunChannel@h
    ori r3, r3, RunChannel@l
    mtctr r3
    bctrl

InitCPU:
    // Check for real mode
    mfmsr r3
    rlwinm r3, r3, 0, 26, 27
    cmpwi r3, 0x30
    beqlr

    // Set DPM, NHR, ICFI, DCFI, DCFA, BTIC, BHT
    lis r3, 0x0011
    ori r3, r3, 0x0c64
    mthid0 r3

    // Set FP
    li r7, 0x2000
    mtmsr r7

    // Set ICE, DCE
    isync
    sync
    ori r3, r3, 0xc000
    mthid0 r3

    // Clear L2E
    sync
    mfl2cr r3
    clrlwi r3, r3, 1
    mtl2cr r3
    sync

    // Set L2I
    oris r4, r3, 0x0020
    mtl2cr r4

WhileL2IP:
    mfl2cr r4
    andi. r4, r4, 0x0001
    bne WhileL2IP

    // Clear L2I and set L2E
    rlwinm r3, r3, 0, 11, 9
    oris r3, r3, 0x8000
    mtl2cr r3

    li r3, 0x0
    mtspr DBAT0U, r3
    mtspr DBAT1U, r3
    mtspr DBAT2U, r3
    mtspr DBAT3U, r3
    mtspr DBAT4U, r3
    mtspr DBAT5U, r3
    mtspr DBAT6U, r3
    mtspr DBAT7U, r3
    mtspr IBAT0U, r3
    mtspr IBAT1U, r3
    mtspr IBAT2U, r3
    mtspr IBAT3U, r3
    mtspr IBAT4U, r3
    mtspr IBAT5U, r3
    mtspr IBAT6U, r3
    mtspr IBAT7U, r3
    isync

    mtsr 0, r3
    mtsr 1, r3
    mtsr 2, r3
    mtsr 3, r3
    mtsr 4, r3
    mtsr 5, r3
    mtsr 6, r3
    mtsr 7, r3
    mtsr 8, r3
    mtsr 9, r3
    mtsr 10, r3
    mtsr 11, r3
    mtsr 12, r3
    mtsr 13, r3
    mtsr 14, r3
    mtsr 15, r3
    isync

    // Configure the low 16 MiB of cached MEM1
    lis r3, 0x8000
    ori r3, r3, 0x01ff
    li r4, 0x0002
    mtspr DBAT0L, r4
    mtspr DBAT0U, r3
    isync
    mtspr IBAT0L, r4
    mtspr IBAT0U, r3
    isync

    // Configure the high 8 MiB of cached MEM1
    lis r3, 0x8100
    ori r3, r3, 0x00ff
    oris r4, r4, 0x0100
    mtspr DBAT2L, r4
    mtspr DBAT2U, r3
    isync
    mtspr IBAT2L, r4
    mtspr IBAT2U, r3
    isync

    // Configure the 64 MiB of cached MEM2
    lis r5, 0x9000
    ori r5, r5, 0x07ff
    lis r6, 0x1000
    ori r6, r6, 0x0002
    mtspr DBAT4L, r6
    mtspr DBAT4U, r5
    isync

    // Configure the 256 MiB of uncached MEM1 and MMIO
    lis r3, 0xc000
    ori r3, r3, 0x1fff
    li r4, 0x002a
    mtspr DBAT1L, r4
    mtspr DBAT1U, r3
    isync

    // Configure the 64 MiB of uncached MEM2
    oris r5, r5, 0xd000
    ori r6, r6, 0x002a
    mtspr DBAT5L, r6
    mtspr DBAT5U, r5
    isync

    // Set SBE
    lis r3, 0x8200
    mtspr 1011, r3

    // Set the default system call exception handler
    lis r3, 0x4c00
    ori r3, r3, 0x0064
    li r4, 0x0000
    stw r3, 0xc00 (r4)

    // Exit real mode
    mflr r3
    oris r3, r3, 0x8000
    mtsrr0 r3
    ori r3, r7, 0x0030
    mtsrr1 r3
    isync
    rfi

    // clang-format on
}
#pragma pop
#endif
