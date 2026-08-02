#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"
#include "portable/online/ItemEvent.hh"

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
        s16 velZ;
        Array<u16, KartCharacterCount> itemFrames;
        Array<u8, KartCharacterCount> itemIDs;
        u8 itemEventCounter;
        u8 itemEventCount;
        Array<ItemEvent, MaxItemEventCount> itemEvents;
        u8 lap;
        u32 time;
    };

    struct Info {
        u16 frame;
        u16 clientFrame;
        u8 kartFlags;
        u8 kartCount;
        Array<Kart, MaxRoomKartCount> karts;
        u16 endFrame;
    };

    struct Result {
        u8 kartIndex;
        u16 points;
    };

    bool ok;
    Optional<Info> info;
    u8 resultCount;
    Array<Result, MaxRoomKartCount> results;
};
