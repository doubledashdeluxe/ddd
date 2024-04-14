#pragma once

#include <common/Types.hh>

template <typename T>
struct TVec2 {
    T x;
    T y;
};

struct Vec2f : TVec2<f32> {};
