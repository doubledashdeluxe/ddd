#pragma once

class SceneType {
public:
    enum {
        Title = 0x00,
        Menu = 0x01,
        Option = 0x02,
        CourseSelect = 0x03,
        PackSelect = 0x04, // Modified
        Record = 0x05,
        Map = 0x06,
        GhostLoadSave = 0x07,
        LanEntry = 0x08,
        Secret = 0x09,
        None = 0x0b,
        HowManyPlayers = 0x0d,  // Added
        NameSelect = 0x0e,      // Added
        TandemSelect = 0x0f,    // Added
        ServerSelect = 0x10,    // Added
        RoomTypeSelect = 0x11,  // Added
        RoomCodeEnter = 0x12,   // Added
        ModeSelect = 0x13,      // Added
        FormatSelect = 0x14,    // Added
        PersonalRoom = 0x15,    // Added
        PlayerList = 0x16,      // Added
        CharacterSelect = 0x17, // Added
        MapSelect = 0x18,       // Moved
        CoursePoll = 0x19,      // Added
        Count = 0x1a,
    };

private:
    SceneType();
};
