#include "KartPad.hh"

#include "game/KartCtrl.hh"
#include "game/SystemRecord.hh"

void KartPad::updateForRearView(u32 index) {
    KartGamePad *kartGamePad = KartCtrl::Instance()->getDriveCont(index);
    s16 padPort = kartGamePad->padPort();
    if (padPort < 0) {
        return;
    }

    u32 masks[4] = {PAD_BUTTON_X, PAD_BUTTON_Y, PAD_TRIGGER_L, PAD_TRIGGER_R};
    u32 mask = masks[(SystemRecord::Instance().m_rearViewButtons >> (padPort * 2)) % 4];
    m_itemMask = (PAD_BUTTON_Y | PAD_BUTTON_X) & ~mask;
    m_lMask = PAD_TRIGGER_L & ~mask;
    m_rMask = PAD_TRIGGER_R & ~mask;
}
