#include "payload/Patcher.hh"

#pragma section RX "first"

extern "C" __declspec(weak) void (*_ctors)() = nullptr;

extern "C" __declspec(section "first") void Start() {
    for (void (**ctor)() = &_ctors; *ctor; ctor++) {
        (*ctor)();
    }

    Patcher::Run();
}
