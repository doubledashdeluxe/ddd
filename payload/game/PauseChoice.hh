#pragma once

class PauseChoice {
public:
    enum {
        // ...
        Title = 0x08,
        // ...
        None = 0x0d,
        PersonalRoom = 0x0e, // Added
        PlayerList = 0x0f,   // Added
        OnlineReplay = 0x10, // Added
    };

private:
    PauseChoice();
};
