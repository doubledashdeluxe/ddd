extern "C" {
#include "OSInterrupt.h"

#include "dolphin/OSThread.h"
#include "dolphin/OSTime.h"
}

#include <payload/Replace.h>

extern "C" u32 userInterruptMask;
extern "C" u32 osInterruptMask;

extern "C" volatile u32 intsr;
extern "C" volatile u32 intmr;

extern "C" volatile u16 miirqflag;

extern "C" volatile u16 dspcsr;

extern "C" volatile u32 exi0cpr;
extern "C" volatile u32 exi1cpr;
extern "C" volatile u32 exi2cpr;

extern "C" volatile u32 aicr;

static const u32 InterruptPriorities[12] = {
        0x100,
        0x40,
        0xf8000000,
        0x200,
        0x80,
        0x10,
        0x3000,
        0x20,
        0x3ff8c00,
        0x40000000,
        0x4000,
        0xffffffff,
};

extern "C" void REPLACED(OSInterruptInit)();
extern "C" REPLACE void OSInterruptInit() {
    REPLACED(OSInterruptInit)();

    OSMaskInterrupts(1 << (31 - 27));
}

extern "C" u32 REPLACED(OSSetInterruptMask)(u32 mask, u32 interrupts);
extern "C" REPLACE u32 OSSetInterruptMask(u32 mask, u32 interrupts) {
#ifdef __CWCC__
    if (__cntlzw(mask) < 17) {
#else
    if (__builtin_clz(mask) < 17) {
#endif
        return REPLACED(OSSetInterruptMask)(mask, interrupts);
    }

    u32 cachedIntmr = 0;
    cachedIntmr |= 1 << 7;
    cachedIntmr |= 1 << 6;
    cachedIntmr |= 1 << 5;
    cachedIntmr |= 1 << 4;
    if (!(interrupts & 1 << (31 - 17))) {
        cachedIntmr |= 1 << 11;
    }
    if (!(interrupts & 1 << (31 - 20))) {
        cachedIntmr |= 1 << 3;
    }
    if (!(interrupts & 1 << (31 - 21))) {
        cachedIntmr |= 1 << 2;
    }
    if (!(interrupts & 1 << (31 - 22))) {
        cachedIntmr |= 1 << 1;
    }
    if (!(interrupts & 1 << (31 - 23))) {
        cachedIntmr |= 1 << 0;
    }
    if (!(interrupts & 1 << (31 - 24))) {
        cachedIntmr |= 1 << 8;
    }
    if (!(interrupts & 1 << (31 - 25))) {
        cachedIntmr |= 1 << 12;
    }
    if (!(interrupts & 1 << (31 - 18))) {
        cachedIntmr |= 1 << 9;
    }
    if (!(interrupts & 1 << (31 - 19))) {
        cachedIntmr |= 1 << 10;
    }
    if (!(interrupts & 1 << (31 - 26))) {
        cachedIntmr |= 1 << 13;
    }
    if (!(interrupts & 1 << (31 - 27))) {
        cachedIntmr |= 1 << 14;
    }
    intmr = cachedIntmr;

    mask &= ~(1 << (31 - 17));
    mask &= ~(1 << (31 - 20));
    mask &= ~(1 << (31 - 21));
    mask &= ~(1 << (31 - 22));
    mask &= ~(1 << (31 - 23));
    mask &= ~(1 << (31 - 24));
    mask &= ~(1 << (31 - 25));
    mask &= ~(1 << (31 - 18));
    mask &= ~(1 << (31 - 19));
    mask &= ~(1 << (31 - 26));
    mask &= ~(1 << (31 - 27));
    return mask;
}

extern "C" REPLACE void OSDispatchInterrupt(u8 /* exception */, OSContext *context) {
    u32 cachedIntsr = intsr;
    u32 cachedIntmr = intmr;

    if (!(cachedIntsr & cachedIntmr & ~(1 << 16))) {
        OSLoadContext(context);
    }

    u32 interrupts = 0;
    if (cachedIntsr & 1 << 7) {
        u16 cachedMiirqflag = miirqflag;
        if (cachedMiirqflag & 1 << 0) {
            interrupts |= 1 << (31 - 0);
        }
        if (cachedMiirqflag & 1 << 1) {
            interrupts |= 1 << (31 - 1);
        }
        if (cachedMiirqflag & 1 << 2) {
            interrupts |= 1 << (31 - 2);
        }
        if (cachedMiirqflag & 1 << 3) {
            interrupts |= 1 << (31 - 3);
        }
        if (cachedMiirqflag & 1 << 4) {
            interrupts |= 1 << (31 - 4);
        }
    }
    if (cachedIntsr & 1 << 6) {
        u16 cachedDspcsr = dspcsr;
        if (cachedDspcsr & 1 << 3) {
            interrupts |= 1 << (31 - 5);
        }
        if (cachedDspcsr & 1 << 5) {
            interrupts |= 1 << (31 - 6);
        }
        if (cachedDspcsr & 1 << 7) {
            interrupts |= 1 << (31 - 7);
        }
    }
    if (cachedIntsr & 1 << 5) {
        u32 cachedAicr = aicr;
        if (cachedAicr & 1 << 3) {
            interrupts |= 1 << (31 - 8);
        }
    }
    if (cachedIntsr & 1 << 4) {
        u32 cachedExi0cpr = exi0cpr;
        if (cachedExi0cpr & 1 << 1) {
            interrupts |= 1 << (31 - 9);
        }
        if (cachedExi0cpr & 1 << 3) {
            interrupts |= 1 << (31 - 10);
        }
        if (cachedExi0cpr & 1 << 11) {
            interrupts |= 1 << (31 - 11);
        }
        u32 cachedExi1cpr = exi1cpr;
        if (cachedExi1cpr & 1 << 1) {
            interrupts |= 1 << (31 - 12);
        }
        if (cachedExi1cpr & 1 << 3) {
            interrupts |= 1 << (31 - 13);
        }
        if (cachedExi1cpr & 1 << 11) {
            interrupts |= 1 << (31 - 14);
        }
        u32 cachedExi2cpr = exi2cpr;
        if (cachedExi2cpr & 1 << 1) {
            interrupts |= 1 << (31 - 15);
        }
        if (cachedExi2cpr & 1 << 3) {
            interrupts |= 1 << (31 - 16);
        }
    }
    if (cachedIntsr & 1 << 13) {
        interrupts |= 1 << (31 - 26);
    }
    if (cachedIntsr & 1 << 12) {
        interrupts |= 1 << (31 - 25);
    }
    if (cachedIntsr & 1 << 10) {
        interrupts |= 1 << (31 - 19);
    }
    if (cachedIntsr & 1 << 9) {
        interrupts |= 1 << (31 - 18);
    }
    if (cachedIntsr & 1 << 8) {
        interrupts |= 1 << (31 - 24);
    }
    if (cachedIntsr & 1 << 3) {
        interrupts |= 1 << (31 - 20);
    }
    if (cachedIntsr & 1 << 2) {
        interrupts |= 1 << (31 - 21);
    }
    if (cachedIntsr & 1 << 1) {
        interrupts |= 1 << (31 - 22);
    }
    if (cachedIntsr & 1 << 11) {
        interrupts |= 1 << (31 - 17);
    }
    if (cachedIntsr & 1 << 0) {
        interrupts |= 1 << (31 - 23);
    }
    if (cachedIntsr & 1 << 14) {
        interrupts |= 1 << (31 - 27);
    }

    interrupts &= ~(userInterruptMask | osInterruptMask);

    if (!interrupts) {
        OSLoadContext(context);
    }

    u32 interrupt;
    for (const u32 *priority = InterruptPriorities;; priority++) {
        if (interrupts & *priority) {
#ifdef __CWCC__
            interrupt = __cntlzw(interrupts & *priority);
#else
            interrupt = __builtin_clz(interrupts & *priority);
#endif
            break;
        }
    }
    OSInterruptHandler handler = OSGetInterruptHandler(interrupt);

    if (!handler) {
        OSLoadContext(context);
    }

    if (interrupt > 4) {
        OSLastInterrupt = interrupt;
        OSLastInterruptTime = OSGetTime();
        OSLastInterruptSrr0 = context->srr0;
    }

    OSDisableScheduler();
    handler(interrupt, context);
    OSEnableScheduler();
    OSReschedule();
    OSLoadContext(context);
}
