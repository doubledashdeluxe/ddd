#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

enum {
    PAD_BUTTON_LEFT = 1 << 0,
    PAD_BUTTON_RIGHT = 1 << 1,
    PAD_BUTTON_DOWN = 1 << 2,
    PAD_BUTTON_UP = 1 << 3,
    PAD_TRIGGER_Z = 1 << 4,
    PAD_TRIGGER_R = 1 << 5,
    PAD_TRIGGER_L = 1 << 6,
    PAD_BUTTON_A = 1 << 8,
    PAD_BUTTON_B = 1 << 9,
    PAD_BUTTON_X = 1 << 10,
    PAD_BUTTON_Y = 1 << 11,
    PAD_BUTTON_START = 1 << 12,
};

enum {
    PAD_ERR_NONE = 0,
    PAD_ERR_NO_CONTROLLER = -1,
};

typedef struct PADStatus {
    u16 button;
    s8 stickX;
    s8 stickY;
    s8 substickX;
    s8 substickY;
    u8 triggerLeft;
    u8 triggerRight;
    u8 analogA;
    u8 analogB;
    s8 err;
} PADStatus;
static_assert(sizeof(PADStatus) == 0xc);

u32 REPLACED(PADRead)(PADStatus *status);
REPLACE u32 PADRead(PADStatus *status);
void REPLACED(PADControlMotor)(int chan, u32 command);
REPLACE void PADControlMotor(int chan, u32 command);
