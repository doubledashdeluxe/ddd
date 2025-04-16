#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

typedef struct {
    s32 mode;
    u8 _04[0x38 - 0x04];
    u8 macaddr[6];
    u8 _3e[0xa8 - 0x3e];
} IPInterface;
size_assert(IPInterface, 0xa8);

BOOL REPLACED(IFInit)(s32 mode);
REPLACE BOOL IFInit(s32 mode);
BOOL IFMute(BOOL mute);

extern IPInterface IFDefault;
