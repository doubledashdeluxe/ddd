#pragma once

#include "game/SoundID.hh"

#include <common/Types.hh>

namespace GameAudio {

class Main {
public:
    bool isWaveLoaded(s32 id);
    void startSequenceBgm(u32 soundID);
    u32 getPlayingSequenceID();
    void startSystemSe(u32 soundID);
    void setMasterVolume(s8 masterVolume);
    void fadeOutAll(u32 duration);
    void setOutputMode(u32 outputMode);

    static Main *Instance();

private:
    Main();

    u8 _00[0xa8 - 0x00];

    static Main *s_instance;
};
size_assert(Main, 0xa8);

} // namespace GameAudio
