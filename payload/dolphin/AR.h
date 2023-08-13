#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

REPLACE u32 ARInit(u32 *stack_index_addr, u32 num_entries);
REPLACE u32 ARAlloc(u32 length);
REPLACE u32 ARGetSize(void);
