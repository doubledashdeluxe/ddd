#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

typedef struct {
    u8 _00[0x0c - 0x00];
    u32 iramMMEMAddr;
    u8 _10[0x18 - 0x10];
    u32 dramMMEMAddr;
    u8 _1c[0x50 - 0x1c];
} DSPTaskInfo;
size_assert(DSPTaskInfo, 0x50);

DSPTaskInfo *REPLACED(DSPAddTask)(DSPTaskInfo *taskInfo);
REPLACE DSPTaskInfo *DSPAddTask(DSPTaskInfo *taskInfo);
void REPLACED(DSPAddPriorTask)(DSPTaskInfo *taskInfo);
REPLACE void DSPAddPriorTask(DSPTaskInfo *taskInfo);
