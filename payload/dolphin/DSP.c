#include "DSP.h"

static void Relocate(u32 *addr) {
    if (*addr < 0x00400000) {
        *addr += 0x400000;
    }
}

DSPTaskInfo *DSPAddTask(DSPTaskInfo *taskInfo) {
    Relocate(&taskInfo->iramMMEMAddr);
    Relocate(&taskInfo->dramMMEMAddr);

    return REPLACED(DSPAddTask)(taskInfo);
}

void DSPAddPriorTask(DSPTaskInfo *taskInfo) {
    Relocate(&taskInfo->iramMMEMAddr);
    Relocate(&taskInfo->dramMMEMAddr);

    REPLACED(DSPAddPriorTask)(taskInfo);
}
