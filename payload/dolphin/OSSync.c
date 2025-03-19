#include <common/Types.h>
#include <payload/Replace.h>

void SystemCallVector();

REPLACE void OSInitSystemCall(void) {
    uintptr_t vector = (uintptr_t)SystemCallVector | 0xc0000000;
    u32 *insts = (u32 *)0xc0000c00;
    for (u32 i = 0; i < 64; i++) {
        insts[i] = i < 7 ? ((u32 *)vector)[i] : 0x00000000;
    }
}
