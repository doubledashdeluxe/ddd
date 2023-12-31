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
        MapSelect = 0xd, // Moved
        Max = 0xe,
    };

private:
    SceneType();
};
