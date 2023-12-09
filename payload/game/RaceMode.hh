#pragma once

class RaceMode {
public:
    enum {
        TA = 0x1,
        GP = 0x2,
        VS = 0x3,
        Balloon = 0x4,
        Robbery = 0x5,
        Bomb = 0x6,
        Escape = 0x7,
        Award = 0x8,
    };

private:
    RaceMode();
};
