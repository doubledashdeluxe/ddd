#pragma once

#include "portable/Array.hh"
#include "portable/Ring.hh"
#include "portable/crypto/Types.hh"

#include <formats/Online.hh>

struct ClientStateServerWriteInfo {
    struct Player {
        u8 profile;
        Array<char, PlayerNameLength + 1> name;
    };

    u8 frameRate;
    char region;
    Array<char, MaxPlatformLength + 1> platform;
    u8 playerCount;
    Array<Player, MaxClientPlayerCount> players;
    u8 kartCount;
    Ring<Hash, MaxCourseCount> raceCourses;
    Ring<Hash, MaxCourseCount> battleCourses;
};
