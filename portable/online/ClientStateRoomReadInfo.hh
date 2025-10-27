#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"
#include "portable/online/RoomOptions.hh"

struct ClientStateRoomReadInfo {
    struct Player {
        Array<char, 4> name;
    };

    struct Kart {
        u8 playerCount;
        Array<Player, 2> players;
    };

    struct Info {
        u8 kartCount;
        Array<Kart, 8> karts;
        u16 spectatorCount;
        u8 modeIndex;
        Array<u8, 32> packHash;
        u64 roomCode;
        u32 spectatingCounter;
        bool spectating;
        bool isRace;
        RoomOptions options;
        u8 entryIndex;
    };

    bool ok;
    Optional<Info> info;
};
