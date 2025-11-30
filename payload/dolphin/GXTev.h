#pragma once

#include <portable/Types.h>

enum {
    GX_TEV_ADD = 0,
};

enum {
    GX_TB_ZERO = 0,
};

enum {
    GX_CS_SCALE_1 = 0,
};

enum {
    GX_TEVPREV = 0,
};

void GXSetTevColorOp(u32 stage, u32 op, u32 bias, u32 scale, BOOL clamp, u32 outReg);
void GXSetTevAlphaOp(u32 stage, u32 op, u32 bias, u32 scale, BOOL clamp, u32 outReg);
