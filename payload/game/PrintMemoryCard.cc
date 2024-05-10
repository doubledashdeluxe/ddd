#include "PrintMemoryCard.hh"

#include "game/GameAudioMain.hh"

s32 PrintMemoryCard::getFinalChoice() const {
    if (m_state == 2 && (m_frame == 0 || m_frame == 25)) {
        return m_finalChoice;
    }

    return 2;
}

void PrintMemoryCard::ack() {
    _c = false;
    _d = false;
    _e = false;
}

void PrintMemoryCard::ack(u32 soundID) {
    if (!_d) {
        GameAudio::Main::Instance()->startSystemSe(soundID);
    }
    ack();
}
