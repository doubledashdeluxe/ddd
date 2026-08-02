#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"

#include <formats/Online.hh>

struct ClientStatePollWriteInfo {
    struct Kart {
        Array<u8, KartCharacterCount> characterIDs;
        u8 kartID;
    };

    struct Ready {
        u8 kartCount;
        Array<Kart, MaxClientKartCount> karts;
        Optional<u8> courseIndex;
    };

    u8 matchIndex;
    u8 packCourseCount;
    u8 kartCount;
    Optional<Ready> ready;
};
