#include "PadMgr.hh"

#include "game/KartGamePad.hh"
#include "game/PadRecorder.hh"
#include "game/SystemRecord.hh"

void PadMgr::SetRecorder(PadRecorder *recorder) {
    s_recorder = recorder;
}

void PadMgr::GetPadData(u8 port, bool remote, KartPadData *data) {
    REPLACED(GetPadData)(port, remote, data);
    if (!remote) {
        u8 rearViewButton = (SystemRecord::Instance().m_rearViewButtons >> (port * 2)) % 4;
        data->buttons &= ~(1 << (2 + rearViewButton));
    }
}

void PadMgr::ProcessKartPad() {
    for (u32 i = 0; i < 16; i++) {
        KartPadData data;
        REPLACED(GetPadData)(i, s_kartPadInput, &data);
        if (s_recorder) {
            s_recorder->line(i, &data);
        }
        KartGamePad::KartPad(i)->expand(data);
    }
    if (s_recorder) {
        s_recorder->framework();
    }
}
