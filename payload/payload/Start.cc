#include "payload/Patcher.hh"
#include "payload/Payload.hh"

#pragma section RX "first"

extern "C" __declspec(section "first") void Start() {
    void (**ctorsStart)() = reinterpret_cast<void (**)()>(Payload::CtorsSectionStart());
    void (**ctorsEnd)() = reinterpret_cast<void (**)()>(Payload::CtorsSectionEnd());
    for (void (**ctor)() = ctorsStart; ctor < ctorsEnd; ctor++) {
        (*ctor)();
    }

    Patcher::Run();
}
