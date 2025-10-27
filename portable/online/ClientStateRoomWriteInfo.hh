#pragma once

#include "portable/Array.hh"
#include "portable/online/RoomOptions.hh"

struct ClientStateRoomWriteInfo {
    u8 modeIndex;
    bool isRace;
    Array<u8, 32> packHash;
    bool isHost;
    u32 roomCounter;
    u64 roomCode;
    u32 spectatingCounter;
    bool spectating;
    RoomOptions options;
    u8 entryIndex;
};
