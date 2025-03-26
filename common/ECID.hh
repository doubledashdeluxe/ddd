#pragma once

#include <common/Types.hh>

struct ECID {
    static ECID Get();

    u32 u;
    u32 m;
    u32 l;
};
