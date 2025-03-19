#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

void REPLACED(GXCallDisplayList)(const void *data, u32 size);
REPLACE void GXCallDisplayList(const void *data, u32 size);
