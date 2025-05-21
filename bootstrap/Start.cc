#include "bootstrap/Bootstrap.hh"
#include "bootstrap/BootstrapBinary.hh"

#include <cube/Arena.hh>
#include <cube/Platform.hh>
extern "C" {
#include <cube/StackCanary.h>

#include <string.h>
}

typedef void (*ChannelEntryFunc)();

extern "C" const u8 channel[];

extern "C" u32 stackTop;

extern "C" void RunBootstrap() {
    void *bssStart = BootstrapBinary::BssSectionStart();
    size_t bssSize = BootstrapBinary::BssSectionSize();
    memset(bssStart, 0, bssSize);

    void (**ctorsStart)() = static_cast<void (**)()>(BootstrapBinary::CtorsSectionStart());
    void (**ctorsEnd)() = static_cast<void (**)()>(BootstrapBinary::CtorsSectionEnd());
    for (void (**ctor)() = ctorsStart; ctor < ctorsEnd; ctor++) {
        (*ctor)();
    }

    if (Platform::IsGameCube()) {
        ChannelEntryFunc channelEntry =
                reinterpret_cast<ChannelEntryFunc>(const_cast<u8 *>(channel + 0x100));
        channelEntry();
    } else {
        MEM1Arena::Init();
        MEM2Arena::Init(0x90000000, 0x93400000);
        Bootstrap::Run();
    }

    while (true) {} // We should never get there
}

#ifdef __CWCC__
extern "C" asm void Start() {
    // clang-format off

    nofralloc

    // Initialize the stack pointer
    lis r1, stackTop@h
    ori r1, r1, stackTop@l

    // Initialize the stack canary
    bl StackCanary_Init

    // Jump to C++ code
    bl RunBootstrap

#ifndef DOLPHIN_FORCE_GAMECUBE
    // Dummy: the presence of such an instruction is the criterion for Dolphin to load an ELF in Wii
    // mode instead of GameCube.
    mtspr 1011, r3
#endif

    // clang-format on
}
#endif
