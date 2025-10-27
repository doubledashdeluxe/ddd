#pragma once

#include <portable/Array.hh>
#include <portable/online/RoomOptions.hh>

class RoomOption {
public:
    enum {
        CodeType = 0,
        Format = 1,
        EngineSize = 2,
        ItemMode = 3,
        LapCount = 4,
        RaceCount = 5,
        BattleCount = 6,
        CourseSelection = 7,
        StageSelection = 8,
        Count = 9,
    };

    static u32 DefaultValue(u32 roomOption);
    static u32 PrevValue(u32 roomOption, u32 value);
    static u32 NextValue(u32 roomOption, u32 value);
    static u32 Read(u32 roomOption, const RoomOptions &options);
    static void Write(u32 roomOption, RoomOptions &options, u32 value);

private:
    typedef u8 RoomOptions::*Accessor;

    struct Option {
        u32 count;
        const u8 *values;
        u32 defaultValue;
        Accessor accessor;
    };

    RoomOption();

    static Array<Option, Count> MakeOptions();
    static bool Index(const Option &option, u32 value, u32 &index);

    static Array<Option, Count> s_options;
};
