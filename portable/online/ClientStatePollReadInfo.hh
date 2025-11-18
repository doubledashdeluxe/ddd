#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"
#include "portable/Ring.hh"

#include <formats/Online.hh>

struct ClientStatePollReadInfo {
    struct Kart {
        Array<u8, KartCharacterCount> characterIDs;
        u8 kartID;
        u8 courseIndex;
    };

    struct Ready {
        u8 kartCount;
        Array<Kart, MaxRoomKartCount> karts;
        u8 kartIndex;
    };

    bool ok;
    Ring<u8, MaxRoomKartCount> kartIndices;
    Optional<Ready> ready;
};
