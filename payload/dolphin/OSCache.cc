extern "C" {
#include "OSCache.h"
}

#include <portable/Align.hh>

extern "C" {
#include <string.h>
}

void LCEnable(void) {}

void LCDisable(void) {}

u32 LCStoreData(void *destAddr, void *srcAddr, u32 nBytes) {
    nBytes = AlignUp(nBytes, 0x20);
    memcpy(destAddr, srcAddr, nBytes);
    return AlignUp(nBytes, 0x1000) / 0x1000;
}

void LCQueueWait(u32 /* len */) {}
