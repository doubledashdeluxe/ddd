#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"
#include "portable/Ring.hh"

#include <formats/Online.hh>

struct ClientStateUpdateReadInfo {
    struct Info {
        u32 size;
        Array<char, MaxUpdateChangelogLength + 1> changelog;
    };

    struct Chunk {
        u16 index;
        Array<u8, UpdateChunkSize> chunk;
    };

    struct Data {
        Ring<Chunk, MaxUpdateIndexCount> chunks;
    };

    Array<char, MaxVersionLength + 1> version;
    Optional<Info> info;
    Data data;
};
