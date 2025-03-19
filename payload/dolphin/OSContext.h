#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

typedef struct OSContext {
    u32 gprs[32];
    u8 _080[0x198 - 0x080];
    u32 srr0;
    u32 srr1;
    u8 _1a0[0x2c8 - 0x1a0];
} OSContext;
size_assert(OSContext, 0x2c8);

REPLACE void OSSetCurrentContext(OSContext *context);
void OSLoadContext(OSContext *context);
