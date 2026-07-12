#pragma once

#include <portable/Types.h>

enum {
    OS_RESET_RESTART = 0,
    OS_RESET_HOTRESET = 1,
    OS_RESET_SHUTDOWN = 2,
};

void OSResetSystem(s32 reset, u32 resetCode, BOOL forceMenu);
