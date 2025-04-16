#pragma once

#include <portable/Types.hh>

class KartPad {
public:
    void updateForRearView(u32 index);

private:
    u8 _00[0x0c - 0x00];
    u32 m_itemMask;
    u8 _10[0x1c - 0x10];
    u32 m_lMask;
    u32 m_rMask;
    u8 _24[0x58 - 0x24];
};
