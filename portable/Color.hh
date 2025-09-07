#pragma once

#include <portable/Types.hh>

struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

bool operator==(const Color &a, const Color &b);
bool operator!=(const Color &a, const Color &b);
