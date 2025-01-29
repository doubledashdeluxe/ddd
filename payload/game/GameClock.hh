#pragma once

#include <common/Types.hh>

class GameClock {
public:
    static u32 GetTime();
    static void Move();

private:
    GameClock();

    static u32 s_time;
};
size_assert(GameClock, 0x1);
