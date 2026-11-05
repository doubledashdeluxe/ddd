#include "KartGame.hh"

#include "game/KartCtrl.hh"
#include "game/SystemRecord.hh"

void KartGame::doStatus() {
    REPLACED(doStatus)();

    u32 gameStatus = m_body->getGameStatus();
    if (!(gameStatus & 0x4) || gameStatus & 0x8) {
        return;
    }

    KartCtrl *kartCtrl = KartCtrl::Instance();
    u32 index = m_body->getIndex();
    KartGamePad *kartGamePad = kartCtrl->getDriveCont(index);
    s16 padPort = kartGamePad->padPort();
    if (padPort < 0) {
        return;
    }

    for (u32 i = 0; i < 8; i++) {
        KartCam *kartCam = kartCtrl->getKartCam(i);
        if (kartCam && kartCam->getBody() == m_body) {
            u32 masks[8] = {PAD_BUTTON_X, PAD_BUTTON_Y, PAD_TRIGGER_L, PAD_TRIGGER_R,
                    PAD_BUTTON_LEFT, PAD_BUTTON_RIGHT, PAD_BUTTON_DOWN, PAD_BUTTON_UP};
            u32 mask = masks[(SystemRecord::Instance().m_rearViewButtons >> (3 * padPort)) % 8];
            kartCam->m_isRearView = kartGamePad->button().level() & mask;
            break;
        }
    }
}

void KartGame::doChange() {
    REPLACED(doChange)();

    u32 index = m_body->getIndex();
    KartPad *kartPad = KartCtrl::Instance()->getKartPad(index);
    kartPad->updateForRearView(index);
}
