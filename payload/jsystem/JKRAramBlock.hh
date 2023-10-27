#pragma once

#include <common/Types.hh>

class JKRAramBlock {
private:
    u8 _00[0x24 - 0x00];
};
static_assert(sizeof(JKRAramBlock) == 0x24);
