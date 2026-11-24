#pragma once

#include <portable/Types.h>

enum {
    GX_LEQUAL = 3,
};

enum {
    GX_PF_RGBA6_Z24 = 1,
};

enum {
    GX_ZC_LINEAR = 0,
};

void GXSetColorUpdate(BOOL update);
void GXSetAlphaUpdate(BOOL update);
void GXSetZMode(BOOL compare, u32 func, BOOL update);
void GXSetPixelFmt(u32 pixFmt, u32 zFmt);
void GXSetDither(BOOL dither);
void GXSetDstAlpha(BOOL enable, u8 alpha);
