#pragma once

#include <common/Types.hh>

namespace GameAudio {

class Main {
public:
    bool isWaveLoaded(s32 id);

    static Main *Instance();

private:
    Main();

    u8 _00[0xa8 - 0x00];

    static Main *s_instance;
};
static_assert(sizeof(Main) == 0xa8);

} // namespace GameAudio
