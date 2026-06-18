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
    class RecState {
    public:
        enum {
            Stopped = 0,
            Recording = 1,
            Playing = 2,
        };

    private:
        RecState();
    };

    class RecPhase {
    public:
        enum {
            Waiting = 0,
            Ready = 1,
            Running = 2,
        };

    private:
        RecPhase();
    };

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
    u8 _14[0x18 - 0x14];
    u32 m_recState;
    u32 m_recPhase;
    u8 _20[0x24 - 0x20];
    u32 m_nextScene;
    UniquePtr<PadRecorder> m_recorder;
    u8 _2c[0x50 - 0x2c];
    u8 m_recGoalFlags;

    static RaceApp *s_instance;
};
size_assert(RaceApp, 0x54);
