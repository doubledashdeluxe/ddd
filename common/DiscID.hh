#pragma once

#include <common/Types.hh>

struct DiscID {
    static DiscID &Get();
    static bool IsValid();

    char gameID[4];
    char makerID[2];
    u8 _06[0x18 - 0x06];
    u32 wiiMagic;
    u32 gameCubeMagic;
};
size_assert(DiscID, 0x20);
