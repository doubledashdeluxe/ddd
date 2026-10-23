#include <cube/Memory.hh>

#include <payload/DOLBinary.hh>
#include <payload/PayloadBinary.hh>

static bool IsDOLCode(uintptr_t addr) {
    return (addr >= reinterpret_cast<uintptr_t>(DOLBinary::InitSectionStart()) &&
                   addr < reinterpret_cast<uintptr_t>(DOLBinary::InitSectionEnd())) ||
            (addr >= reinterpret_cast<uintptr_t>(DOLBinary::TextSectionStart()) &&
                    addr < reinterpret_cast<uintptr_t>(DOLBinary::TextSectionEnd()));
}

static bool IsPayloadCode(uintptr_t addr) {
    return addr >= reinterpret_cast<uintptr_t>(PayloadBinary::TextSectionStart()) &&
            addr < reinterpret_cast<uintptr_t>(PayloadBinary::TextSectionEnd());
}

bool Memory::IsCode(uintptr_t addr) {
    return IsDOLCode(addr) || IsPayloadCode(addr);
}

uintptr_t Memory::CachedToPhysical(uintptr_t addr) {
    if (addr >= 0x80000000 && addr < 0x80400000) {
        addr += 0x400000;
    }
    return addr & 0x7fffffff;
}

uintptr_t Memory::PhysicalToCached(uintptr_t addr) {
    if (addr >= 0x00400000 && addr < 0x00800000) {
        addr -= 0x400000;
    }
    return addr | 0x80000000;
}
