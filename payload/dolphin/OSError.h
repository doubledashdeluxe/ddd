#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

typedef u16 OSError;

void OSPanic(const char *file, s32 line, const char *format, ...);
void REPLACED(OSReport)(const char *format, ...);
REPLACE void OSReport(const char *format, ...);
