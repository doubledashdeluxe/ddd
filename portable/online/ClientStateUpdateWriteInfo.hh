#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"
#include "portable/Ring.hh"

#include <formats/Online.hh>

struct ClientStateUpdateWriteInfo {
    struct Info {
        char region;
        Array<char, MaxPlatformLength + 1> platform;
        u8 language;
    };

    struct Data {
        Ring<u16, MaxUpdateIndexCount> indices;
    };

    u32 serverIndex;
    Info info;
    Optional<Data> data;
};
