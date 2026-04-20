#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class RaceDirector {
public:
    u32 racePhase() const;
    bool isFrameRenewal() const;
    void calc();
    void calc(s32 adjustment);

private:
    class Race2DMode {
    public:
        enum {
            OneTwo = 0,
            ThreeFour = 1,
        };

    private:
        Race2DMode();
    };

    void REPLACED(resetCommon)();
    REPLACE void resetCommon();
    void doCourseDemo();
    void doNoCourseDemo();
    void doReset();
    void doSafety();
    void REPLACED(doRunning)(bool r4);
    REPLACE void doRunning(bool r4);
    void doWait();
    void doEnding();
    void checkPauseChoice();
    void setPhaseWait(u32 nextPhase, bool fadeDisplay, bool fadeAudio, s32 duration);
    bool checkRaceEnd() const;

    u8 _00[0x04 - 0x00];
    u32 m_race2DMode;
    u8 _08[0x0c - 0x08];
    u32 m_racePhase;
    u8 _10[0x28 - 0x10];
    bool m_isFrameRenewal;
    u8 _29[0x48 - 0x29];
};
size_assert(RaceDirector, 0x48);
