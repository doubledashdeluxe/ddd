#pragma once

#include "game/MinimapConfig.hh"

class Race2D {
public:
    void setMinimapConfig(const MinimapConfig &minimapConfig);

    static Race2D *Instance();

private:
    u8 _0000[0x1110 - 0x0000];
    MinimapConfig m_minimapConfig;
    u8 _1124[0x4ec4 - 0x1124];

    static Race2D *s_instance;
};
size_assert(Race2D, 0x4ec4);
