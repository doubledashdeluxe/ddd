#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

void DCInvalidateRange(void *addr, u32 nBytes);
void DCFlushRange(void *addr, u32 nBytes);
void DCStoreRange(void *addr, u32 nBytes);

void ICInvalidateRange(void *addr, u32 nBytes);

REPLACE void LCEnable(void);
REPLACE void LCDisable(void);
REPLACE u32 LCStoreData(void *destAddr, void *srcAddr, u32 nBytes);
REPLACE void LCQueueWait(u32 len);
