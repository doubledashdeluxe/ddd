#pragma once

#include <common/Types.h>

typedef struct {
    u8 _00[0x18 - 0x00];
} OSMutex;
size_assert(OSMutex, 0x18);

void OSInitMutex(OSMutex *mutex);
void OSLockMutex(OSMutex *mutex);
void OSUnlockMutex(OSMutex *mutex);
