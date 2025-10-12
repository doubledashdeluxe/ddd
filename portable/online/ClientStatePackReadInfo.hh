#pragma once

#include "portable/Array.hh"
#include "portable/Counts.hh"
#include "portable/Optional.hh"

struct ClientStatePackReadInfo {
    struct Pack {
        u16 playerCount;
    };

    Array<Optional<Pack>, MaxPackCount> packs;
};
