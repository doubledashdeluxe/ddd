#pragma once

#include "portable/Ring.hh"
#include "portable/crypto/Types.hh"
#include "portable/online/RoomOptions.hh"

#include <formats/Online.hh>

struct ClientStateRoomWriteInfo {
    bool isSearch;
    u8 modeIndex;
    bool isRace;
    Ring<u8, MaxCourseCount> packCourseIndices;
    Hash packHash;
    bool isHost;
    Array<u8, 16> roomNonce;
    u64 roomCode;
    u32 spectatingCounter;
    bool spectating;
    RoomOptions options;
    u8 entryIndex;
    bool continuing;
};
