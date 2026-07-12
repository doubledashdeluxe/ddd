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
        Update = 0x12,          // Added
        RoomTypeSelect = 0x13,  // Added
        RoomCodeEnter = 0x14,   // Added
        ModeSelect = 0x15,      // Added
        FormatSelect = 0x16,    // Added
        PersonalRoom = 0x17,    // Added
        TeamSelect = 0x18,      // Added
        PlayerList = 0x19,      // Added
        CharacterSelect = 0x1a, // Added
        MapSelect = 0x1b,       // Moved
        CoursePoll = 0x1c,      // Added
        Replay = 0x1d,          // Added
        Count = 0x1e,
    };

private:
    SceneType();
};
