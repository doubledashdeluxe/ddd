#include "WipeManager.hh"

void WipeManager::drawMario(f32 t) {
    GXColor color = {0, 0, 0, 255};
    m_mario->draw(t, color);
}

void WipeManager::drawWipeCurtain(s32 consoleIndex, f32 t) {
    GXColor color = {0, 0, 0, 255};
    m_wipeCurtain->draw(consoleIndex, t, color);
}

void WipeManager::calcWipeCurtain(s32 consoleIndex, f32 t) {
    GXColor color = {0, 0, 0, 255};
    m_wipeCurtain->calc(consoleIndex, t, color);
}

WipeManager *WipeManager::Instance() {
    return s_instance;
}
