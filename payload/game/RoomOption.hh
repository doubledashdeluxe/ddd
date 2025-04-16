#pragma once

#include <portable/Types.hh>

class RoomOption {
public:
    enum {
        Code = 0,
        Format = 1,
        EngineSize = 2,
        ItemMode = 3,
        LapCount = 4,
        RaceCount = 5,
        BattleCount = 6,
        CourseSelection = 7,
        MapSelection = 8,
    };

    static u32 ValueCount(u32 roomOption);

private:
    RoomOption();
};
