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
        ProfileSelect = 0x0e,   // Added
        NameSelect = 0x0f,      // Added
        TandemSelect = 0x10,    // Added
        ServerSelect = 0x11,    // Added
        RoomTypeSelect = 0x12,  // Added
        RoomCodeEnter = 0x13,   // Added
        ModeSelect = 0x14,      // Added
        FormatSelect = 0x15,    // Added
        PersonalRoom = 0x16,    // Added
        TeamSelect = 0x17,      // Added
        PlayerList = 0x18,      // Added
        CharacterSelect = 0x19, // Added
        MapSelect = 0x1a,       // Moved
        CoursePoll = 0x1b,      // Added
        Count = 0x1c,
    };

private:
    SceneType();
};
