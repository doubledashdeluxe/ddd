#pragma once

#include "jsystem/TBox.hh"

#include <common/Types.hh>

class J2DGraphContext {
public:
    J2DGraphContext();
    virtual ~J2DGraphContext();
    virtual void vf_0c();
    virtual void vf_10();
    virtual void setViewport();
    // ...

private:
    u8 _04[0xbc - 0x04];
};
size_assert(J2DGraphContext, 0xbc);
