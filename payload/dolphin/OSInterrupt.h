#pragma once

#include "dolphin/OSContext.h"

#include <portable/Types.h>

typedef void (*OSInterruptHandler)(s16 interrupt, OSContext *context);

BOOL OSDisableInterrupts(void);
BOOL OSRestoreInterrupts(BOOL enable);

OSInterruptHandler OSSetInterruptHandler(s16 interrupt, OSInterruptHandler handler);
OSInterruptHandler OSGetInterruptHandler(s16 interrupt);

u32 OSMaskInterrupts(u32 interrupts);
u32 OSUnmaskInterrupts(u32 interrupts);

extern volatile s16 OSLastInterrupt;
extern volatile u32 OSLastInterruptSrr0;
extern volatile s64 OSLastInterruptTime;
