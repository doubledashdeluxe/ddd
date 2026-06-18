#pragma once

#include "game/KartPadRecord.hh"
#include "game/PadMgr.hh"
#include "game/RaceTime.hh"

#include <portable/Types.hh>

class PadRecorder {
public:
    PadRecorder();
    ~PadRecorder();
    void rec();
    void play();
    void stop();
    void setRecord(u8 port, KartPadRecord *record);
    void finalizeRecord(u8 port, RaceTime totalTime);
    void line(u8 port, PadMgr::KartPadData *data);
    void framework();
    void nextFrame();

private:
    u8 _000[0x188 - 0x000];
};
size_assert(PadRecorder, 0x188);
