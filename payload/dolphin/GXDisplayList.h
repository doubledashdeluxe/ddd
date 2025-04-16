#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

void REPLACED(GXCallDisplayList)(const void *data, u32 size);
REPLACE void GXCallDisplayList(const void *data, u32 size);
