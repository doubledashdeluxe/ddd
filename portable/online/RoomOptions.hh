#pragma once

#include "portable/Types.hh"

struct RoomOptions {
    u8 codeType;
    u8 format;
    u8 engineSize;
    u8 itemMode;
    u8 lapCount;
    u8 matchCount;
    u8 courseSelection;
};

bool operator==(const RoomOptions &a, const RoomOptions &b);
bool operator!=(const RoomOptions &a, const RoomOptions &b);
