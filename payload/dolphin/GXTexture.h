#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

typedef struct GXTexObj {
    u8 _00[0x0c - 0x00];
    u32 image_ptr;
    u8 _10[0x20 - 0x10];
} GXTexObj;
size_assert(GXTexObj, 0x20);

typedef struct GXTlutObj {
    s32 fmt;
    u32 lut;
    u16 n_entries;
    u8 _a[0xc - 0xa];
} GXTlutObj;
size_assert(GXTlutObj, 0xc);

void REPLACED(GXInitTexObj)(GXTexObj *obj, void *image_ptr, u16 width, u16 height, s32 format,
        s32 wrap_s, s32 wrap_t, BOOL mipmap);
REPLACE void GXInitTexObj(GXTexObj *obj, void *image_ptr, u16 width, u16 height, s32 format,
        s32 wrap_s, s32 wrap_t, BOOL mipmap);

REPLACE void GXInitTlutObj(GXTlutObj *tlut_obj, void *lut, s32 fmt, u16 n_entries);
