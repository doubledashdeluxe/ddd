#pragma once

#include "portable/Ring.hh"
#include "portable/crypto/Types.hh"

#include <formats/Online.hh>

class ReplayManager {
public:
    struct Player {
        Array<char, PlayerNameLength + 1> name;
    };

    struct Client {
        Ring<Player, MaxClientPlayerCount> players;
        Ring<u8, MaxClientKartCount> teams;
        u8 kartFlags;
    };

    struct Replay {
        u8 modeIndex;
        u8 packCourseCount;
        Hash packHash;
        u8 courseIndex;
        Ring<Client, MaxReplayClientCount> clients;
        s64 time;
        u8 roomType;
        u8 format;
        u64 roomCode;
        u8 kartCount;
        bool isRace;
        u8 packIndex;
        Array<char, 128> path;
        u32 offset;
    };
};
