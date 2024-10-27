#pragma once

#include <common/Types.hh>

struct KartDB {
    u32 id;
    u8 _04[0x10 - 0x04];
};
size_assert(KartDB, 0x10);
