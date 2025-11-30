#pragma once

#include <jsystem/TBox2.hh>
#include <portable/Types.hh>

struct MinimapConfig {
    TBox2<f32> box;
    u32 orientation;
};
size_assert(MinimapConfig, 0x14);
