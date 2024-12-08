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
        MapSelect = 0xd,        // Moved
        HowManyPlayers = 0xe,   // Added
        NameSelect = 0xf,       // Added
        TandemSelect = 0x10,    // Added
        ServerSelect = 0x11,    // Added
        RoomTypeSelect = 0x12,  // Added
        CharacterSelect = 0x13, // Added
        Count = 0x14,
    };

private:
    SceneType();
};
