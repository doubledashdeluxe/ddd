#include "PadMgr.hh"

#include "game/SystemRecord.hh"

void PadMgr::GetPadData(u8 port, bool remote, KartPadData *data) {
    REPLACED(GetPadData)(port, remote, data);
    if (!remote) {
        u8 rearViewButton = (SystemRecord::Instance().m_rearViewButtons >> (port * 2)) % 4;
        data->buttons &= ~(1 << (2 + rearViewButton));
    }
}
