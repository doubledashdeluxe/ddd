#pragma once

#include "portable/Types.hh"

struct ItemEvent {
    u8 frame;
    s8 stickY;
    u8 itemID;
    s16 posX;
    s16 posZ;
};
