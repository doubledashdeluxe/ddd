#pragma once

#include <common/Types.hh>
#include <jsystem/TBox.hh>

struct MinimapConfig {
    TBox<f32> box;
    u32 orientation;
};
size_assert(MinimapConfig, 0x14);
