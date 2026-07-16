#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

enum {
    GX_VA_POS = 9,
    GX_VA_CLR0 = 11,
    GX_VA_TEX0 = 13,
};

enum {
    GX_NONE = 0,
    GX_DIRECT = 1,
    GX_INDEX8 = 2,
    GX_INDEX16 = 3,
};

enum {
    GX_POS_XY = 0,
    GX_POS_XYZ = 1,
    GX_NRM_XYZ = 0,
    GX_NRM_NBT = 1,
    GX_NRM_NBT3 = 2,
    GX_CLR_RGB = 0,
    GX_CLR_RGBA = 1,
    GX_TEX_S = 0,
    GX_TEX_ST = 1,
};

enum {
    GX_U8 = 0,
    GX_S8 = 1,
    GX_U16 = 2,
    GX_S16 = 3,
    GX_F32 = 4,
    GX_RGB565 = 0,
    GX_RGB8 = 1,
    GX_RGBX8 = 2,
    GX_RGBA4 = 3,
    GX_RGBA6 = 4,
    GX_RGBA8 = 5,
};

enum {
    GX_TG_MTX2x4 = 1,
};

enum {
    GX_TG_TEX0 = 4,
};

enum {
    GX_IDENTITY = 60,
};

enum {
    GX_PTIDENTITY = 125,
};

void GXSetVtxDesc(u32 attr, u32 attrType);

void GXClearVtxDesc(void);
void GXSetVtxAttrFmt(u32 vtxFmt, u32 attr, u8 compCnt, u32 compType, u8 fract);

void REPLACED(GXSetArray)(u32 attr, const void *base, u8 stride);
REPLACE void GXSetArray(u32 attr, const void *base, u8 stride);

void GXInvalidateVtxCache(void);

void GXSetTexCoordGen2(u32 coord, u32 genType, u32 genSrc, u32 mtx, BOOL normalize, u32 postMtx);
