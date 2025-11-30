#pragma once

#include <portable/Types.hh>

class KartDrawer {
public:
    bool isVisible() const;

private:
    u8 _000[0x018 - 0x000];
    u16 m_hiddenFlags;
    u8 _01a[0x178 - 0x01a];
};
size_assert(KartDrawer, 0x178);
