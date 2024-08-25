#pragma once

#include <common/Types.hh>

class GameClock {
public:
    static void Move();

private:
    GameClock();
};
size_assert(GameClock, 0x1);
