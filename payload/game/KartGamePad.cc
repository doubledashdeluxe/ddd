#include "KartGamePad.hh"

s32 KartGamePad::padPort() const {
    return m_padPort;
}

KartGamePad *KartGamePad::GamePad(u32 index) {
    return s_gamePads[index];
}

KartGamePad *KartGamePad::KartPad(u32 index) {
    return s_kartPads[index];
}
