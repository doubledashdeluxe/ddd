#include "loader/Loader.hh"

#include <common/Arena.hh>
extern "C" {
#include <common/StackCanary.h>
}

extern "C" {
#include <string.h>
}

extern "C" u32 stackTop;

extern "C" void RunLoader() {
    void *bssStart = Loader::BssSectionStart();
    size_t bssSize = Loader::BssSectionSize();
    memset(bssStart, 0, bssSize);

    void (**ctorsStart)() = reinterpret_cast<void (**)()>(Loader::CtorsSectionStart());
    void (**ctorsEnd)() = reinterpret_cast<void (**)()>(Loader::CtorsSectionEnd());
    for (void (**ctor)() = ctorsStart; ctor < ctorsEnd; ctor++) {
        (*ctor)();
    }

    MEM1Arena::Init();
    MEM2Arena::Init(0x91000000, 0x93400000);
    Context *context = new (MEM2Arena::Instance(), 0x4) Context;
    Loader::PayloadEntryFunc payloadEntry = Loader::Run(context);
    context->mem2ArenaLo = reinterpret_cast<uintptr_t>(MEM2Arena::Instance()->alloc(0x0, 0x4));
    context->mem2ArenaHi = reinterpret_cast<uintptr_t>(MEM2Arena::Instance()->alloc(0x0, -0x4));
    context->console = Console::Instance();
    if (payloadEntry) {
        (*payloadEntry)(context);
    }
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
    bl RunLoader

    // We should never get there
loop:
    b loop

    // Dummy: the presence of such an instruction is the criterion for Dolphin to load an ELF in Wii
    // mode instead of GameCube.
    mtspr 1011, r3

    // clang-format on
}

void operator delete(void *) {
    while (true) {}
}
#endif
