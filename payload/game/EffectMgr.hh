#pragma once

#include <jsystem/TVec3.hh>

class EffectMgr {
public:
    void createEmt2D(const char *name, const Vec3f &pos);

    static EffectMgr *Instance();

private:
    u8 _00[0x38 - 0x00];

    static EffectMgr *s_instance;
};
size_assert(EffectMgr, 0x38);
