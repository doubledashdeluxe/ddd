#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

enum {
    THP_FAKE_LC_BUFFER_SIZE = 0x3c00,
};

void REPLACED(THPInit)(void);
REPLACE void THPInit(void);
void THPSetFakeLCBuffer(u8 fakeLCBuffer[THP_FAKE_LC_BUFFER_SIZE]);
