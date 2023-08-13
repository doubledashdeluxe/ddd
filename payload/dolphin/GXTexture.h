#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

typedef struct GXTlutObj {
    s32 fmt;
    u32 lut;
    u16 n_entries;
    u8 _a[0xc - 0xa];
} GXTlutObj;
static_assert(sizeof(GXTlutObj) == 0xc);

REPLACE void GXInitTlutObj(GXTlutObj *tlut_obj, void *lut, s32 fmt, u16 n_entries);
