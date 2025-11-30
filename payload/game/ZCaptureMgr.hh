#pragma once

#include <portable/Types.hh>

class ZCaptureMgr {
public:
    void setPosition(u32 nodeIndex, u32 screenIndex, s16 x, s16 y);
    u32 getZValue(u32 nodeIndex, u32 screenIndex);

private:
    u8 _0[0xc - 0x0];
};
size_assert(ZCaptureMgr, 0xc);
