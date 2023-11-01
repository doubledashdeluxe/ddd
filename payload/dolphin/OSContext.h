#pragma once

#include <common/Types.h>

typedef struct OSContext {
    u8 _000[0x198 - 0x000];
    u32 srr0;
    u8 _19c[0x2c8 - 0x19c];
} OSContext;
size_assert(OSContext, 0x2c8);

void OSLoadContext(OSContext *context);
