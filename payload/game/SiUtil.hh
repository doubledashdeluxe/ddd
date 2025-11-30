#pragma once

#include <jsystem/TVec2.hh>
#include <jsystem/TVec3.hh>

class SiUtil {
public:
    static s32 GetScreenPos(s32 camIndex, const Vec3f &worldPos, Vec2f &screenPos);

private:
    SiUtil();
};
