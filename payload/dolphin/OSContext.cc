extern "C" {
#include "OSContext.h"
}

#include <common/Memory.hh>

extern "C" u32 currentContextAddress;
extern "C" OSContext *currentContext;
extern "C" OSContext *currentFPUContext;

#ifdef __CWCC__
void OSSetCurrentContext(OSContext *context) {
    u32 contextAddress = Memory::CachedToPhysical(context);
    currentContextAddress = contextAddress;
    currentContext = context;
    u32 register msr;
    if (currentFPUContext == context) {
        context->srr1 |= 1 << 13;
        asm volatile(mfmsr msr);
        msr |= 1 << 1;
        asm volatile(mtmsr msr);
    } else {
        context->srr1 &= ~(1 << 13);
        asm volatile(mfmsr msr);
        msr &= ~(1 << 13);
        msr |= 1 << 1;
        asm volatile(mtmsr msr);
        asm volatile(isync);
    }
}
#endif
