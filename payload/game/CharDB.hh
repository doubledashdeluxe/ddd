#pragma once

#include <portable/Types.hh>

struct CharDB {
    u16 id;
    u8 _2[0xc - 0x2];
};
size_assert(CharDB, 0xc);
