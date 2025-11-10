#pragma once

#include "portable/Array.hh"

#include <formats/Online.hh>

struct ClientStateServerWriteInfo {
    struct Player {
        u8 profile;
        Array<char, PlayerNameLength + 1> name;
    };

    u8 frameRate;
    u8 playerCount;
    Array<Player, MaxClientPlayerCount> players;
    u8 kartCount;
};
