#pragma once

#include <common/Types.h>

typedef u16 OSError;

void OSPanic(const char *file, s32 line, const char *msg, ...);
void OSReport(const char *msg, ...);
