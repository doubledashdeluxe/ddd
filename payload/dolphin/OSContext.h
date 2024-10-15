#pragma once

#include <common/Types.h>

typedef struct OSContext {
    u32 gprs[32];
    u8 _080[0x198 - 0x080];
    u32 srr0;
    u8 _19c[0x2c8 - 0x19c];
} OSContext;
size_assert(OSContext, 0x2c8);

void OSLoadContext(OSContext *context);
