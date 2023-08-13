#pragma once

#include <common/Types.h>

enum {
    VI_NTSC = 0,
    VI_PAL = 1,
    VI_EURGB60 = 5,
};

enum {
    VI_XFBMODE_SF = 0,
    VI_XFBMODE_DF = 1,
};

void VIInit(void);
void VIFlush(void);
void VIWaitForRetrace(void);

void VISetBlack(BOOL black);
u32 VIGetTvFormat(void);
u32 VIGetDTVStatus(void);
