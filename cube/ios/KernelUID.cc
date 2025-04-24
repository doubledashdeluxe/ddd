// clang-format off
//
// Based on https://github.com/mkwcat/saoirse/blob/master/channel/Main/IOSBoot.cpp
//
// clang-format on

#include "KernelUID.hh"

#include "cube/Clock.hh"
#include "cube/Memory.hh"
#include "cube/Platform.hh"
#include "cube/ios/File.hh"
#include "cube/ios/Resource.hh"

namespace IOS {

KernelUID::KernelUID(bool again) : m_ok(Acquire(again)) {}

KernelUID::~KernelUID() {
    if (m_ok) {
        Release();
    }
}

bool KernelUID::ok() const {
    return m_ok;
}

bool KernelUID::Acquire(bool again) {
    if (Platform::IsDolphin()) {
        return true;
    }

    Resource sha("/dev/sha", Mode::None);
    if (!sha.ok()) {
        return false;
    }

    SafeFlush(ArmCode, sizeof(ArmCode));

    u32 *mem1 = Memory::PhysicalToCached<u32>(0x00000000);
    if (again) {
        // Enter Thumb state
        *mem1++ = 0xfaffffff; // blx 0x4
    }
    *mem1++ = 0x4903468d; // ldr r1, =0x10100000; mov sp, r1;
    *mem1++ = 0x49034788; // ldr r1, =entrypoint; blx r1;
    // Overwrite reserved handler to loop infinitely
    *mem1++ = 0x49036209; // ldr r1, =0xffff0014; str r1, [r1, #0x20];
    *mem1++ = 0x47080000; // bx r1
    *mem1++ = 0x10100000; // Temporary stack
    *mem1++ = Memory::CachedToPhysical(ArmCode);
    *mem1++ = 0xffff0014; // Reserved handler

    alignas(0x20) Resource::IoctlvPair pairs[4];
    pairs[0].data = nullptr;
    pairs[0].size = 0;
    pairs[1].data = Memory::PhysicalToCached<void>(0x7ffe0028);
    pairs[1].size = 0;
    // Unused vector utilized for cache safety
    pairs[2].data = Memory::PhysicalToCached<void>(0x00000000);
    pairs[2].size = 0x40;

    // This should never return an error if the exploit succeeded
    if (sha.ioctlv(0, 1, 2, pairs) < 0) {
        return false;
    }

    while (ReadMessage(1) != 1) {
        Clock::WaitMilliseconds(1);
    }
    return true;
}

void KernelUID::Release() {
    if (Platform::IsDolphin()) {
        return;
    }

    WriteMessage(1, 2);
    while (ReadMessage(1) != 3) {
        Clock::WaitMilliseconds(1);
    }
}

void KernelUID::SafeFlush(const void *start, size_t size) {
    // The IPC function flushes the cache here on PPC, and then IOS invalidates its own cache.
    // Note: IOS doesn't check for the invalid fd before doing what we want.
    File file(-1);
    file.write(start, size);
}

#ifdef __CWCC__
u32 KernelUID::ReadMessage(u32 index) {
    u32 address = Memory::CachedToPhysical(&ArmCode[index]);
    u32 message;
    asm volatile("lwz %0, 0x0 (%1); sync" : "=r"(message) : "b"(0xc0000000 | address));
    return message;
}

void KernelUID::WriteMessage(u32 index, u32 message) {
    u32 address = Memory::CachedToPhysical(&ArmCode[index]);
    asm volatile("stw %0, 0x0 (%1); eieio" : : "r"(message), "b"(0xc0000000 | address));
}
#endif

u32 KernelUID::Syscall(u32 id) {
    return 0xe6000010 | id << 5;
}

// clang-format off
const u32 KernelUID::ArmCode[] = {
    /* 0x00 */ 0xea000000, // b       0x8
    /* 0x04 */ 0x00000000, // MESSAGE_VALUE
    // Set PPC UID to kernel
    /* 0x08 */ 0xe3a0000f, // mov     r0, #15
    /* 0x0C */ 0xe3a01000, // mov     r1, #0
    /* 0x10 */ Syscall(SC::IOS_SetUid),
    // Send response to PPC
    /* 0x14 */ 0xe24f0018, // adr     r0, MESSAGE_VALUE
    /* 0x18 */ 0xe3a01001, // mov     r1, #1
    /* 0x1C */ 0xe5801000, // str     r1, [r0]
    // Flush the response to main memory
    /* 0x20 */ 0xe3a01004, // mov     r1, #4
    /* 0x24 */ Syscall(SC::IOS_FlushDCache),
    // Wait for response back from PPC
    // loop_start:
    /* 0x28 */ 0xe24f002c, // adr     r0, MESSAGE_VALUE
    /* 0x2C */ 0xe5902000, // ldr     r2, [r0]
    /* 0x30 */ 0xe3520002, // cmp     r2, #2
    /* 0x34 */ 0x0a000001, // beq     loop_break
    /* 0x38 */ Syscall(SC::IOS_InvalidateDCache),
    /* 0x3C */ 0xeafffff9, // b       loop_start
    // loop_break:
    // Reset PPC UID back to 15
    /* 0x40 */ 0xe3a0000f, // mov     r0, #15
    /* 0x44 */ 0xe3a0100f, // mov     r1, #15
    /* 0x48 */ Syscall(SC::IOS_SetUid),
    // Send response to PPC
    /* 0x4C */ 0xe24f0050, // adr     r0, MESSAGE_VALUE
    /* 0x50 */ 0xe3a01003, // mov     r1, #3
    /* 0x54 */ 0xe5801000, // str     r1, [r0]
    // Flush the response to main memory
    /* 0x58 */ 0xe3a01004, // mov     r1, #4
    /* 0x5C */ Syscall(SC::IOS_FlushDCache),
    /* 0x60 */ 0xe12fff1e, // bx      lr
};
// clang-format on

} // namespace IOS
