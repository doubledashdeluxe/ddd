#pragma once

#include "game/PadMgr.hh"

#include <portable/Types.hh>

class PadRecorder {
public:
    PadRecorder();
    ~PadRecorder();
    void line(u8 port, PadMgr::KartPadData *data);
    void framework();

private:
    u8 _000[0x188 - 0x000];
};
size_assert(PadRecorder, 0x188);
