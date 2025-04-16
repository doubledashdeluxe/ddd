#pragma once

#include <jsystem/TBox.hh>
#include <portable/Types.hh>

struct MinimapConfig {
    TBox<f32> box;
    u32 orientation;
};
size_assert(MinimapConfig, 0x14);
