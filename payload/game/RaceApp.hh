#pragma once

#include "game/GameApp.hh"
#include "game/PadRecorder.hh"
#include "game/RaceMgr.hh"

#include <payload/Replace.hh>
#include <portable/UniquePtr.hh>

class RaceApp : public GameApp {
public:
    static void Call();
    REPLACE static RaceApp *Create();
    static RaceApp *Instance();

private:
    RaceApp();
    REPLACE ~RaceApp() override;
    void draw() override;
    void REPLACED(calc)();
    REPLACE void calc() override;

    void REPLACED(ctrlRace)();
    REPLACE void ctrlRace();
    void ctrlRecorder();

    UniquePtr<RaceMgr> m_raceMgr;
    u32 m_state;
    u8 _14[0x24 - 0x14];
    u32 m_nextScene;
    UniquePtr<PadRecorder> m_recorder;
    u8 _2c[0x54 - 0x2c];

    static RaceApp *s_instance;
};
size_assert(RaceApp, 0x54);
