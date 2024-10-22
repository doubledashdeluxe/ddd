#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

u32 REPLACED(ARInit)(u32 *stack_index_addr, u32 num_entries);
REPLACE u32 ARInit(u32 *stack_index_addr, u32 num_entries);
u32 REPLACED(ARAlloc)(u32 length);
REPLACE u32 ARAlloc(u32 length);
u32 REPLACED(ARGetSize)(void);
REPLACE u32 ARGetSize(void);
