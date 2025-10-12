#pragma once

#include "portable/Array.hh"
#include "portable/Counts.hh"
#include "portable/Optional.hh"

struct ClientStatePackWriteInfo {
    struct Pack {
        Array<u8, 32> hash;
    };

    u8 modeIndex;
    u32 packCount;
    Array<Pack, MaxPackCount> packs;
    Optional<u32> packIndex;
};
