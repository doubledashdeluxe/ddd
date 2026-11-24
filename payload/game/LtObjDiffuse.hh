#pragma once

#include "game/LightObj.hh"

class LtObjDiffuse : public LightObj {
public:
    ~LtObjDiffuse() override;
    void draw() override;
    // ...

private:
    u8 _38[0x90 - 0x38];
};
size_assert(LtObjDiffuse, 0x90);
