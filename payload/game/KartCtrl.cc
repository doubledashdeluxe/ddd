#include "KartCtrl.hh"

KartPad *KartCtrl::getKartPad(u32 index) {
    return m_kartPads[index];
}

KartCam *KartCtrl::getKartCam(u32 index) {
    return m_kartCams[index];
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
