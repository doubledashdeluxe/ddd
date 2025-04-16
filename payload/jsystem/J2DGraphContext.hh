#pragma once

#include "jsystem/TBox.hh"

#include <portable/Types.hh>

class J2DGraphContext {
public:
    J2DGraphContext();
    virtual ~J2DGraphContext();
    virtual void place(const TBox<f32> &box);
    virtual void place(f32 x, f32 y, f32 w, f32 h);
    virtual void setViewport();
    // ...

private:
    u8 _04[0xbc - 0x04];
};
size_assert(J2DGraphContext, 0xbc);
