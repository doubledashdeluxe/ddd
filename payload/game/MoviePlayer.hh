#pragma once

#include <portable/Types.hh>

class MoviePlayer {
public:
    static MoviePlayer *Instance();
    static void DrawDone();

private:
    MoviePlayer();
    virtual ~MoviePlayer();

    u8 _04[0x48 - 0x04];

    static MoviePlayer *s_instance;
};
size_assert(MoviePlayer, 0x48);
