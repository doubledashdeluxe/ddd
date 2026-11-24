#pragma once

#include "game/LtObjDiffuse.hh"

class RaceKartLight : public LtObjDiffuse {
public:
    ~RaceKartLight() override;
    void vf_0c() override;
    void draw() override;
    // ...

private:
    u8 _090[0x128 - 0x090];
};
size_assert(RaceKartLight, 0x128);
