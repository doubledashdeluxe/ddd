#pragma once

#include "portable/Array.hh"
#include "portable/Ring.hh"

#include <formats/Online.hh>

struct ClientStateRaceWriteInfo {
    struct Inputs {
        u8 inputCount;
        Array<u16, MaxKartPlayerCount> inputs;
    };

    struct Kart {
        Ring<Inputs, MaxKartInputCount> inputs;
        s16 posX;
        s16 posY;
        s16 posZ;
        s8 angle;
        s16 velX;
        s16 velY;
        s16 velZ;
    };

    u16 frame;
    u8 kartCount;
    Array<Kart, MaxClientKartCount> karts;
};
