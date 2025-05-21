#include "PadMgr.hh"

#include "game/KartGamePad.hh"
#include "game/PadRecorder.hh"
#include "game/SystemRecord.hh"

void PadMgr::GetPadData(u8 port, bool remote, KartPadData *data) {
    REPLACED(GetPadData)(port, remote, data);
    if (!remote) {
        u8 rearViewButton = (SystemRecord::Instance().m_rearViewButtons >> (port * 2)) % 4;
        data->buttons &= ~(1 << (2 + rearViewButton));
    }
}

void PadMgr::ProcessKartPad() {
    PadRecorder *recorder = PadRecorder::Instance();
    for (u32 i = 0; i < 16; i++) {
        KartPadData data;
        REPLACED(GetPadData)(i, s_kartPadInput, &data);
        if (recorder) {
            recorder->line(i, &data);
        }
        KartGamePad::KartPad(i)->expand(data);
    }
    if (recorder) {
        recorder->framework();
    }
}
