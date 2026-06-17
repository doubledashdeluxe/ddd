#pragma once

#include "game/ItemObj.hh"

class KartPad {
public:
    void updateForRearView(u32 index);

private:
    u8 _00[0x0c - 0x00];
    u32 m_itemMask;
    u8 _10[0x1c - 0x10];
    u32 m_lMask;
    u32 m_rMask;
    u8 _24[0x3c - 0x24];

public:
    f32 m_itemStickY;

private:
    u8 _40[0x4c - 0x40];

public:
    u32 m_goldenMushroomFrame;
    u8 : 7;
    bool m_hasGoldenMushroom : 1;
    u8 m_itemFrame;

private:
    u8 _52[0x54 - 0x52];

public:
    ItemObj *m_goldenMushroom;
};
size_assert(KartPad, 0x58);
