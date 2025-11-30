#pragma once

#include "game/GameApp.hh"

#include <payload/Replace.hh>

class RaceApp : public GameApp {
public:
    static void Call();
    static RaceApp *Instance();

private:
    void REPLACED(ctrlRace)();
    REPLACE void ctrlRace();

    u8 _0c[0x24 - 0x0c];
    u32 m_nextScene;
    u8 _28[0x54 - 0x28];

    static RaceApp *s_instance;
};
size_assert(RaceApp, 0x54);
