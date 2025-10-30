#pragma once

#include "portable/Array.hh"

#include <formats/Online.hh>

struct ClientStateServerWriteInfo {
    struct Player {
        u8 profile;
        Array<char, 4> name;
    };

    u8 playerCount;
    Array<Player, MaxClientPlayerCount> players;
    u8 kartCount;
};
