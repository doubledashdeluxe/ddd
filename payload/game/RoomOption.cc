#include "RoomOption.hh"

#include <formats/Online.hh>
#include <portable/Algorithm.hh>

u32 RoomOption::DefaultValue(u32 roomOption) {
    return s_options[roomOption].defaultValue;
}

u32 RoomOption::PrevValue(u32 roomOption, u32 value) {
    const Option &option = s_options[roomOption];
    u32 index;
    if (!Index(option, value, index)) {
        return option.defaultValue;
    }
    if (index > 0) {
        index--;
    } else {
        index = option.count - 1;
    }
    return option.values ? option.values[index] : index;
}

u32 RoomOption::NextValue(u32 roomOption, u32 value) {
    const Option &option = s_options[roomOption];
    u32 index;
    if (!Index(option, value, index)) {
        return option.defaultValue;
    }
    if (index < option.count - 1) {
        index++;
    } else {
        index = 0;
    }
    return option.values ? option.values[index] : index;
}

u32 RoomOption::Read(u32 roomOption, const RoomOptions &options) {
    return options.*s_options[roomOption].accessor;
}

void RoomOption::Write(u32 roomOption, RoomOptions &options, u32 value) {
    options.*s_options[roomOption].accessor = value;
}

void Write(u32 roomOption, RoomOptions &options, u32 value);

Array<RoomOption::Option, RoomOption::Count> RoomOption::MakeOptions() {
    Array<Option, Count> options;
    for (u32 i = 0; i < options.count(); i++) {
        options[i].values = nullptr;
        options[i].defaultValue = 0;
    }
    options[CodeType].count = RoomOptionCodeType::Count;
    options[CodeType].accessor = &RoomOptions::codeType;
    options[Format].count = RoomOptionFormat::Count;
    options[Format].accessor = &RoomOptions::format;
    options[EngineSize].count = RoomOptionEngineSize::Count;
    options[EngineSize].defaultValue = RoomOptionEngineSize::Large;
    options[EngineSize].accessor = &RoomOptions::engineSize;
    options[ItemMode].count = RoomOptionItemMode::Count;
    options[ItemMode].accessor = &RoomOptions::itemMode;
    options[LapCount].count = MaxLapCount + 1;
    static const u8 matchCounts[] = {1, 2, 3, 4, 5, 6, 8, 16, 24, 32, 48, 64, 96};
    options[LapCount].accessor = &RoomOptions::lapCount;
    options[RaceCount].count = ::Count(matchCounts);
    options[RaceCount].values = matchCounts;
    options[RaceCount].defaultValue = DefaultMatchCount;
    options[RaceCount].accessor = &RoomOptions::matchCount;
    options[BattleCount].count = ::Count(matchCounts);
    options[BattleCount].values = matchCounts;
    options[BattleCount].defaultValue = DefaultMatchCount;
    options[BattleCount].accessor = &RoomOptions::matchCount;
    options[CourseSelection].count = RoomOptionCourseSelection::Count;
    options[CourseSelection].accessor = &RoomOptions::courseSelection;
    options[StageSelection].count = RoomOptionCourseSelection::Count;
    options[StageSelection].accessor = &RoomOptions::courseSelection;
    return options;
}

bool RoomOption::Index(const Option &option, u32 value, u32 &index) {
    u32 count = option.count;
    const u8 *values = option.values;
    if (values) {
        for (u32 i = 0; i < count; i++) {
            if (values[i] == value) {
                index = i;
                return true;
            }
        }
    } else if (value < count) {
        index = value;
        return true;
    }
    return false;
}

Array<RoomOption::Option, RoomOption::Count> RoomOption::s_options = MakeOptions();
