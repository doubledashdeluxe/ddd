#pragma once

#include "portable/Array.hh"
#include "portable/Counts.hh"
#include "portable/Optional.hh"
#include "portable/crypto/Types.hh"

struct ClientStatePackWriteInfo {
    struct Pack {
        Hash hash;
    };

    u8 modeIndex;
    u32 packCount;
    Array<Pack, MaxPackCount> packs;
    Optional<u32> packIndex;
};
