#pragma once

#include <portable/Types.hh>

class LightObj {
public:
    virtual ~LightObj();
    virtual void vf_0c();
    virtual void draw();
    // ...

private:
    u8 _04[0x38 - 0x04];
};
size_assert(LightObj, 0x38);
