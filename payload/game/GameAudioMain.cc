#include "GameAudioMain.hh"

namespace GameAudio {

Main *GameAudio::Main::Instance() {
    return s_instance;
}

} // namespace GameAudio
