#pragma once

#include <portable/Types.hh>

class LightMgr {
public:
    void draw();

    static LightMgr *Instance();

private:
    u8 _00[0x2c - 0x00];

    static LightMgr *s_instance;
};
size_assert(LightMgr, 0x2c);
