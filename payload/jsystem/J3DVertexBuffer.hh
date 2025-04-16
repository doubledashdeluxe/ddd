#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class J3DVertexBuffer {
public:
    REPLACE void setArray();

private:
    u8 _00[0x2c - 0x00];
    void *_2c;
    void *_30;
    void *_34;
};
size_assert(J3DVertexBuffer, 0x38);
