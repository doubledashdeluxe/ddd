#include "PrintMemoryCard.hh"

#include "game/GameAudioMain.hh"

void PrintMemoryCard::ack(u32 soundID) {
    if (!_d) {
        GameAudio::Main::Instance()->startSystemSe(soundID);
    }
    _c = false;
    _d = false;
    _e = false;
}
