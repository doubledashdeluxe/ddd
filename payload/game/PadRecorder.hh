#pragma once

#include "game/PadMgr.hh"

#include <portable/Types.hh>

class PadRecorder {
public:
    void line(u8 port, PadMgr::KartPadData *data);
    void framework();

    static PadRecorder *Instance();

private:
    u8 _000[0x188 - 0x000];

    static PadRecorder *s_instance;
};
size_assert(PadRecorder, 0x188);
