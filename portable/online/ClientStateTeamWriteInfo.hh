#pragma once

#include "portable/Array.hh"

#include <formats/Online.hh>

struct ClientStateTeamWriteInfo {
    bool isHost;
    u8 kartCount;
    Array<u8, MaxRoomKartCount> kartTeams;
    u8 entryIndex;
    u8 teamCount;
};
