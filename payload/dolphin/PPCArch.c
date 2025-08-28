#include "PPCArch.h"

u32 PPCMfhid2(void) {
    u32 hid2 = REPLACED(PPCMfhid2)();
    hid2 |= 1 << 28; // LCE
    return hid2;
}
