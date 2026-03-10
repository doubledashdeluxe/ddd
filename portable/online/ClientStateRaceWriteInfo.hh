#pragma once

#include "portable/Array.hh"
#include "portable/Ring.hh"

#include <formats/Online.hh>

struct ClientStateRaceWriteInfo {
    typedef Array<u16, MaxKartPlayerCount> Inputs;

    struct Kart {
        u8 inputCount;
        Ring<Inputs, MaxKartInputCount> inputs;
        u8 driver;
        s16 posX;
        s16 posY;
        s16 posZ;
        s8 angle;
        s16 velX;
        s16 velY;
        s16 velZ;
        Array<u16, KartCharacterCount> itemFrames;
        u8 rank;
    };

    u16 frame;
    u8 kartCount;
    Array<Kart, MaxClientKartCount> karts;
    Array<u16, 16> itemCounts;
};
