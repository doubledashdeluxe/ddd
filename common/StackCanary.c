#include "StackCanary.h"

#define IABR 1010
#define IABR_BE_BIT 30

// clang-format off
asm void StackCanary_Init(void) {
    nofralloc

    mftb      r0
    rlwinm    r0, r0, 0, IABR_BE_BIT + 1, IABR_BE_BIT - 1
    clrlwi    r0, r0, 8
    oris      r0, r0, 0x8000

    mtspr     IABR, r0
    blr
} 

asm u32 StackCanary_Canary(void) {
    nofralloc

    mfspr     r3, IABR
    blr
}
// clang-format on
