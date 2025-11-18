#pragma once

#include "portable/crypto/Types.hh"
#include "portable/online/RoomOptions.hh"

struct ClientStateRoomWriteInfo {
    u8 modeIndex;
    bool isRace;
    u8 packCourseCount;
    Hash packHash;
    bool isHost;
    u32 roomCounter;
    u64 roomCode;
    u32 spectatingCounter;
    bool spectating;
    RoomOptions options;
    u8 entryIndex;
    bool continuing;
};
