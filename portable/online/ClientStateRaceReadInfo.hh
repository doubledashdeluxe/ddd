#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"

#include <formats/Online.hh>

struct ClientStateRaceReadInfo {
    struct Kart {
        u16 frame;
        u8 inputCount;
        Array<u16, MaxKartPlayerCount> inputs;
        u8 driver;
        s16 posX;
        s16 posY;
        s16 posZ;
        s8 angle;
        s16 velX;
        s16 velY;
        s16 velZ;
        Array<u16, KartCharacterCount> itemFrames;
        Array<u8, KartCharacterCount> itemIDs;
    };

    struct Info {
        u16 frame;
        u16 clientFrame;
        u8 kartFlags;
        u8 kartCount;
        Array<Kart, MaxRoomKartCount> karts;
    };

    bool ok;
    Optional<Info> info;
};
