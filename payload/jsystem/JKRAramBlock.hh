#pragma once

#include <common/Types.hh>

class JKRAramBlock {
private:
    u8 _00[0x24 - 0x00];
};
size_assert(JKRAramBlock, 0x24);
