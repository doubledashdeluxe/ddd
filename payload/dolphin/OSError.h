#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

void OSPanic(const char *file, s32 line, const char *format, ...);
void REPLACED(OSReport)(const char *format, ...);
REPLACE void OSReport(const char *format, ...);
