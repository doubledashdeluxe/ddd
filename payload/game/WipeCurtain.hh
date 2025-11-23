#pragma once

#include "game/WipeBase.hh"

class WipeCurtain : public WipeBase {
public:
    // ...

private:
    u8 _1c[0x2c - 0x1c];
};
size_assert(WipeCurtain, 0x2c);
