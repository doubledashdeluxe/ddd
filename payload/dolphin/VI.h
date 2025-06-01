#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

enum {
    VI_NTSC = 0,
    VI_PAL = 1,
    VI_EURGB60 = 5,
};

enum {
    VI_XFBMODE_SF = 0,
    VI_XFBMODE_DF = 1,
};

typedef void (*VIRetraceCallback)(u32 retraceCount);

VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback callback);
VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback callback);
void REPLACED(VIInit)(void);
REPLACE void VIInit(void);
void VIWaitForRetrace(void);
void VIFlush(void);
void REPLACED(VISetNextFrameBuffer)(void *fb);
REPLACE void VISetNextFrameBuffer(void *fb);
void REPLACED(VISetBlack)(BOOL black);
REPLACE void VISetBlack(BOOL black);
u32 VIGetRetraceCount(void);
u32 VIGetTvFormat(void);
u32 VIGetDTVStatus(void);
