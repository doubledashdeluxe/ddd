#include "KartGamePad.hh"

KartGamePad *KartGamePad::GamePad(u32 index) {
    return s_gamePads[index];
}

KartGamePad *KartGamePad::KartPad(u32 index) {
    return s_kartPads[index];
}
