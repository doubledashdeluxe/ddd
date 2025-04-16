#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class Race2DParam {
public:
    REPLACE void setX(f32 x);
    REPLACE void setY(f32 y);
    REPLACE void setS(f32 s);

private:
    u8 _00[0x0c - 0x00];
    f32 m_x;
    f32 m_y;
    f32 m_s;
};
size_assert(Race2DParam, 0x18);
