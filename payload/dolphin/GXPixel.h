#pragma once

#include <portable/Types.h>

enum {
    GX_PF_RGBA6_Z24 = 1,
};

enum {
    GX_ZC_LINEAR = 0,
};

void GXSetPixelFmt(u32 pixFmt, u32 zFmt);
void GXSetDither(BOOL dither);
