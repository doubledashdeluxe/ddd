#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class RaceDirector {
public:
    u32 racePhase() const;
    bool isFrameRenewal() const;
    bool raceEnd() const;
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
    void REPLACED(doEnding)();
    REPLACE void doEnding();
    void REPLACED(checkPauseChoice)();
    REPLACE void checkPauseChoice();
    void setPhaseWait(u32 nextPhase, bool fadeDisplay, bool fadeAudio, s32 duration);
    bool REPLACED(checkRaceEnd)() const;
    REPLACE bool checkRaceEnd() const;

    u8 _00[0x04 - 0x00];
    u32 m_race2DMode;
    u8 _08[0x0c - 0x08];
    u32 m_racePhase;
    u8 _10[0x20 - 0x10];
    u32 m_frame;
    u8 _24[0x28 - 0x24];
    bool m_isFrameRenewal;
    u8 _29[0x2a - 0x29];
    bool m_raceEnd;
    u8 _2b[0x34 - 0x2b];
    u32 m_endingState;
    u8 _38[0x48 - 0x38];
};
size_assert(RaceDirector, 0x48);
