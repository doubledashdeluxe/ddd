#include "payload/Patcher.hh"
#include "payload/Payload.hh"

extern "C" {
#include <dolphin/DB.h>
#include <dolphin/OS.h>

#include <stdlib.h>
}

#pragma section RX "first"

extern "C" void __init_registers(void);
extern "C" void __init_hardware(void);
extern "C" void __init_data(void);
extern "C" void __init_user(void);
extern "C" int main(void);

extern "C" void RunPayload() {
    void (**ctorsStart)() = reinterpret_cast<void (**)()>(Payload::CtorsSectionStart());
    void (**ctorsEnd)() = reinterpret_cast<void (**)()>(Payload::CtorsSectionEnd());
    for (void (**ctor)() = ctorsStart; ctor < ctorsEnd; ctor++) {
        (*ctor)();
    }

    Patcher::Run();

    DBInit();
    OSInit();

    Payload::Run();
}

extern "C" __declspec(section "first") asm void Start() {
    // clang-format off

    nofralloc

    bl __init_registers

    li r0, -0x1
    stwu r1, -0x8(r1)
    stw r0, 0x4(r1)
    stw r0, 0x0(r1)

    bl __init_hardware
    bl __init_data

    bl RunPayload

    bl __init_user
    bl main
    bl exit

    // clang-format on
}
