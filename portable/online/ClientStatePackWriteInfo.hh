#pragma once

#include "portable/Array.hh"
#include "portable/Counts.hh"

struct ClientStatePackWriteInfo {
    struct Pack {
        Array<u8, 32> hash;
    };

    u8 modeIndex;
    u32 packCount;
    Array<Pack, MaxPackCount> packs;
};
