#pragma once

class SceneType {
public:
    enum {
        Title = 0x0,
        Menu = 0x1,
        Option = 0x2,
        CourseSelect = 0x3,
        PackSelect = 0x4, // Modified
        Record = 0x5,
        Map = 0x6,
        GhostLoadSave = 0x7,
        LanEntry = 0x8,
        Secret = 0x9,
        None = 0xb,
        HowManyPlayers = 0xd,   // Added
        NameSelect = 0xe,       // Added
        TandemSelect = 0xf,     // Added
        ServerSelect = 0x10,    // Added
        ModeSelect = 0x11,      // Added
        FormatSelect = 0x12,    // Added
        PlayerList = 0x13,      // Added
        CharacterSelect = 0x14, // Added
        MapSelect = 0x15,       // Moved
        CoursePoll = 0x16,      // Added
        Count = 0x17,
    };

private:
    SceneType();
};
