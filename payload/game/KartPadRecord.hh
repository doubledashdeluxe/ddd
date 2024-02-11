#pragma once

#include <common/Types.hh>

struct KartPadRecord {
    u8 _0000[0x000c - 0x0000];
    s32 frame;
    u8 _0010[0x8bb8 - 0x0010];
};
size_assert(KartPadRecord, 0x8bb8);
