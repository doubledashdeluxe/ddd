#pragma once

#include "portable/Array.hh"
#include "portable/Counts.hh"
#include "portable/Optional.hh"
#include "portable/crypto/Types.hh"

struct ClientStatePackWriteInfo {
    struct Pack {
        u8 courseCount;
        Hash hash;
    };

    bool isDuel;
    u8 modeIndex;
    u32 packCount;
    Array<Pack, MaxPackCount> packs;
    Optional<u32> packIndex;
};
