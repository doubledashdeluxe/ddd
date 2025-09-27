#pragma once

#include "dolphin/OSContext.h"

#include <payload/Replace.h>
#include <portable/Types.h>

enum {
    EXI_READ = 0,
    EXI_WRITE = 1,
    EXI_RDWR = 2,
};

typedef void (*EXICallback)(s32 chan, OSContext *context);

BOOL EXISelect(s32 chan, u32 dev, u32 freq);
BOOL EXIDeselect(s32 chan);
void EXISetExiInterruptMask(s32 chan);
EXICallback EXISetExiCallback(s32 chan, EXICallback exiCallback);
void EXIProbeReset(void);
BOOL EXIAttach(s32 chan, EXICallback extCallback);
BOOL EXIDetach(s32 chan);
BOOL REPLACED(EXIDma)(s32 chan, void *buf, s32 len, u32 type, EXICallback callback);
REPLACE BOOL EXIDma(s32 chan, void *buf, s32 len, u32 type, EXICallback callback);
REPLACE BOOL EXIImm(s32 chan, void *buf, s32 len, u32 type, EXICallback callback);
BOOL EXIImmEx(s32 chan, void *buf, s32 len, u32 type);
BOOL EXISync(s32 chan);
BOOL EXIClearInterrupts(s32 chan, BOOL exi, BOOL tc, BOOL ext);

BOOL EXILock(s32 chan, u32 dev, EXICallback unlockedCallback);
BOOL EXIUnlock(s32 chan);

s32 EXIGetID(s32 chan, u32 dev, u32 *id);
s32 REPLACED(EXIGetType)(s32 chan, u32 dev, u32 *type);
REPLACE s32 EXIGetType(s32 chan, u32 dev, u32 *type);
