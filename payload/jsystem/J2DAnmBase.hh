#pragma once

#include <common/Types.hh>

class J2DScreen;

class J2DAnmBase {
public:
    J2DAnmBase();
    virtual ~J2DAnmBase();
    virtual void searchUpdateMaterialID(J2DScreen *screen);

private:
    u8 _04[0x08 - 0x04];

public:
    f32 m_frame;

private:
    u8 _0c[0x10 - 0x0c];
};
size_assert(J2DAnmBase, 0x10);
