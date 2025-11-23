#pragma once

#include "game/Mario.hh"
#include "game/WipeCurtain.hh"

class WipeManager {
public:
    void drawMario(f32 t);
    void drawWipeCurtain(s32 consoleIndex, f32 t);
    void calcWipeCurtain(s32 consoleIndex, f32 t);

    static WipeManager *Instance();

private:
    u8 _00[0xac - 0x00];
    Mario *m_mario;
    u8 _b0[0xcc - 0xb0];
    WipeCurtain *m_wipeCurtain;
    u8 _d0[0xd4 - 0xd0];

    static WipeManager *s_instance;
};
size_assert(WipeManager, 0xd4);
