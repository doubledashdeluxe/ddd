#pragma once

#include <common/Types.h>

enum {
    EXI_READ = 0,
    EXI_WRITE = 1,
};

BOOL EXISelect(s32 chan, u32 dev, u32 freq);
BOOL EXIDeselect(s32 chan);
void EXIProbeReset(void);
BOOL EXIImmEx(s32 chan, void *buf, s32 len, u32 type);

BOOL EXILock(s32 chan, u32 dev, void *unlockedCallback);
BOOL EXIUnlock(s32 chan);
