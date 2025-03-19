#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class J3DModelData {
private:
    REPLACE void syncJ3DSysPointers();

    u8 _00[0x6c - 0x00];
    void *_6c;
    u8 _70[0xa0 - 0x70];
    void *_a0;
    void *_a4;
    u8 _a8[0xa8 - 0xa4];
    void *_ac;
    u8 _b0[0xe4 - 0xb0];
};
size_assert(J3DModelData, 0xe4);
