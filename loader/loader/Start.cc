#include "loader/Loader.hh"

#include <common/Types.hh>

extern "C" __declspec(weak) void (*_ctors)() = nullptr;

extern "C" u32 stackTop;

extern "C" void RunLoader() {
    for (void (**ctor)() = &_ctors; *ctor; ctor++) {
        (*ctor)();
    }

    Apploader::GameEntryFunc gameEntry = Loader::Run();
    if (gameEntry) {
        (*gameEntry)();
    }
}

extern "C" asm void Start() {
    // clang-format off

    nofralloc

    // Initialize the stack pointer
    lis r1, stackTop@h
    ori r1, r1, stackTop@l

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
