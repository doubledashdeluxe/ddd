#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

void REPLACED(GXSetArray)(u32 attr, const void *base, u8 stride);
REPLACE void GXSetArray(u32 attr, const void *base, u8 stride);
