#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

void REPLACED(GXCopyDisp)(void *dest, BOOL clear);
REPLACE void GXCopyDisp(void *dest, BOOL clear);
