#pragma once

#include "portable/Array.hh"

struct ClientStateServerWriteInfo {
    struct Player {
        u8 profile;
        Array<char, 4> name;
    };

    u8 playerCount;
    Array<Player, 4> players;
};
