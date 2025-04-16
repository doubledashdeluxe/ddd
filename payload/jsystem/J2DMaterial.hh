#pragma once

#include <portable/Types.hh>

class J2DMaterial {
public:
    J2DMaterial();
    virtual ~J2DMaterial();

    void animation();

private:
    u8 _04[0x88 - 0x04];
};
size_assert(J2DMaterial, 0x88);
