#pragma once

#include "portable/Array.hh"
#include "portable/Counts.hh"
#include "portable/Optional.hh"

#include <formats/Online.hh>

struct ClientStatePackReadInfo {
    struct Pack {
        u16 playerCount;
        Array<u16, FormatCount> formatPlayerCounts;
    };

    Array<Optional<Pack>, MaxPackCount> packs;
    Optional<u32> packIndex;
};
