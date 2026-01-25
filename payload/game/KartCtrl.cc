#include "KartCtrl.hh"

KartGamePad *KartCtrl::getKartGamePad(u32 kartIndex, u32 playerIndex) const {
    return m_kartGamePads[kartIndex][playerIndex];
}

KartPad *KartCtrl::getKartPad(u32 kartIndex) const {
    return m_kartPads[kartIndex];
}

KartBody *KartCtrl::getKartBody(u32 kartIndex) const {
    return m_kartBodies[kartIndex];
}

KartCam *KartCtrl::getKartCam(u32 kartIndex) const {
    return m_kartCams[kartIndex];
}

void KartCtrl::dynamicsInit(bool r4) {
    REPLACED(dynamicsInit)(r4);

    for (u32 i = 0; i < m_kartCount; i++) {
        m_kartPads[i]->updateForRearView(i);
    }
}

void KartCtrl::dynamicsReset() {
    REPLACED(dynamicsReset)();

    for (u32 i = 0; i < m_kartCount; i++) {
        m_kartPads[i]->updateForRearView(i);
    }
}

KartCtrl *KartCtrl::Instance() {
    return s_instance;
}
