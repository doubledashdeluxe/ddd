#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

void REPLACED(GXSetArray)(u32 attr, const void *base, u8 stride);
REPLACE void GXSetArray(u32 attr, const void *base, u8 stride);
