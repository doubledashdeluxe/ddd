#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"

#include <formats/Online.hh>

struct ClientStateTeamReadInfo {
    struct Info {
        u8 kartCount;
        Array<u8, MaxRoomKartCount> kartTeams;
        u8 entryIndex;
        bool continuing;
    };

    bool ok;
    Optional<Info> info;
};
