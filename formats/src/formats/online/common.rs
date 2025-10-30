use crate::complex_data_type::ComplexDataType;
use crate::simple_data_type::SimpleDataType;
use crate::simple_enum_type::SimpleEnumType;
use crate::struct_type::StructType;
use crate::unit_type::UnitType;

pub fn mode_index() -> impl ComplexDataType {
    SimpleEnumType::new("ModeIndex")
        .with_variant("Versus")
        .with_variant("Balloon")
        .with_variant("Escape")
        .with_variant("Bomb")
        .with_variant("TimeAttack")
}

pub fn room_options_race() -> impl ComplexDataType {
    let lap_count: SimpleDataType<u8> = SimpleDataType::new();
    let match_count: SimpleDataType<u8> = SimpleDataType::new();
    let entry_index: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("RoomOptionsRace")
        .with_field("race", UnitType)
        .with_field("code_type", room_option_code_type())
        .with_field("format", room_option_format())
        .with_field("engine_size", room_option_engine_size())
        .with_field("item_mode", room_option_item_mode())
        .with_field("lap_count", lap_count)
        .with_field("match_count", match_count)
        .with_field("course_selection", room_option_course_selection())
        .with_field("entry_index", entry_index)
}

pub fn room_options_battle() -> impl ComplexDataType {
    let match_count: SimpleDataType<u8> = SimpleDataType::new();
    let entry_index: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("RoomOptionsBattle")
        .with_field("battle", UnitType)
        .with_field("code_type", room_option_code_type())
        .with_field("format", room_option_format())
        .with_field("item_mode", room_option_item_mode())
        .with_field("match_count", match_count)
        .with_field("course_selection", room_option_course_selection())
        .with_field("entry_index", entry_index)
}

pub fn room_option_code_type() -> impl ComplexDataType {
    SimpleEnumType::new("RoomOptionCodeType").with_variant("Long").with_variant("Short")
}

pub fn room_option_format() -> impl ComplexDataType {
    SimpleEnumType::new("RoomOptionFormat")
        .with_variant("FreeForAll")
        .with_variant("TeamsOf2")
        .with_variant("TeamsOf4")
}

pub fn room_option_engine_size() -> impl ComplexDataType {
    SimpleEnumType::new("RoomOptionEngineSize")
        .with_variant("Small")
        .with_variant("Medium")
        .with_variant("Large")
        .with_variant("Mirror")
}

pub fn room_option_item_mode() -> impl ComplexDataType {
    SimpleEnumType::new("RoomOptionItemMode")
        .with_variant("Recommended")
        .with_variant("Basic")
        .with_variant("Frantic")
        .with_variant("None")
}

pub fn room_option_course_selection() -> impl ComplexDataType {
    SimpleEnumType::new("RoomOptionCourseSelection")
        .with_variant("Poll")
        .with_variant("Host")
        .with_variant("Random")
}

pub const MAX_VERSION_LENGTH: u8 = 19;
pub const MIN_CLIENT_PLAYER_COUNT: u8 = 1;
pub const MAX_CLIENT_PLAYER_COUNT: u8 = 4;
pub const PLAYER_NAME_LENGTH: u8 = 3;
pub const MODE_INDEX_COUNT: u8 = 5;
