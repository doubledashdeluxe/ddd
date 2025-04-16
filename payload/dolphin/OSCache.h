#pragma once

#include <portable/Types.h>

void DCInvalidateRange(void *addr, u32 nBytes);
void DCFlushRange(void *addr, u32 nBytes);
void DCStoreRange(void *addr, u32 nBytes);

void ICInvalidateRange(void *addr, u32 nBytes);
