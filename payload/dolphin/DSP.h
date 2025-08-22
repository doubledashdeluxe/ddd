#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

typedef struct {
    u8 _00[0x0c - 0x00];
    u32 iramMMEMAddr;
    u8 _10[0x18 - 0x10];
    u32 dramMMEMAddr;
    u8 _1c[0x50 - 0x1c];
} DSPTaskInfo;
size_assert(DSPTaskInfo, 0x50);

u32 DSPCheckMailToDSP(void);
void DSPSendMailToDSP(u32 mail);
DSPTaskInfo *REPLACED(DSPAddTask)(DSPTaskInfo *taskInfo);
REPLACE DSPTaskInfo *DSPAddTask(DSPTaskInfo *taskInfo);
void REPLACED(DSPAddPriorTask)(DSPTaskInfo *taskInfo);
REPLACE void DSPAddPriorTask(DSPTaskInfo *taskInfo);
