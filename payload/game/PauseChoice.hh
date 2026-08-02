#pragma once

class PauseChoice {
public:
    enum {
        // ...
        Title = 0x8,
        // ...
        None = 0xd,
        PersonalRoom = 0xe, // Added
        PlayerList = 0xf,   // Added
    };

private:
    PauseChoice();
};
