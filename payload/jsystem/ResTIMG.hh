#pragma once

#include <common/Types.hh>

struct ResTIMG {
    u8 _00[0x20 - 0x00];
};
size_assert(ResTIMG, 0x20);
