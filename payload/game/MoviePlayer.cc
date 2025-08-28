#include "MoviePlayer.hh"

extern "C" {
#include <dolphin/THPDec.h>
}

void MoviePlayer::Create(JKRHeap *heap) {
    REPLACED(THPInit)();
    u8 *fakeLCBuffer = new (heap, 0x20) u8[THP_FAKE_LC_BUFFER_SIZE];
    THPSetFakeLCBuffer(fakeLCBuffer);

    REPLACED(Create)(heap);
}

MoviePlayer *MoviePlayer::Instance() {
    return s_instance;
}
