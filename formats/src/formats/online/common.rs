use crate::complex_data_type::ComplexDataType;
use crate::simple_data_type::SimpleDataType;
use crate::simple_enum_type::SimpleEnumType;
use crate::struct_type::StructType;
use crate::unit_type::UnitType;

pub fn frame_rate() -> impl ComplexDataType {
    SimpleEnumType::new("FrameRate").with_variant("SixtyHz").with_variant("FiftyHz")
}

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
        .with_variant("Duel")
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

pub fn character_id() -> impl ComplexDataType {
    SimpleEnumType::new("CharacterID")
        .with_variant("BabyMario")
        .with_variant("BabyLuigi")
        .with_variant("Patapata")
        .with_variant("Nokonoko")
        .with_variant("Peach")
        .with_variant("Daisy")
        .with_variant("Mario")
        .with_variant("Luigi")
        .with_variant("Wario")
        .with_variant("Waluigi")
        .with_variant("Yoshi")
        .with_variant("Catherine")
        .with_variant("Donkey")
        .with_variant("Diddy")
        .with_variant("Koopa")
        .with_variant("KoopaJr")
        .with_variant("Kinopio")
        .with_variant("Kinopico")
        .with_variant("Teresa")
        .with_variant("Pakkun")
}

pub fn kart_id() -> impl ComplexDataType {
    SimpleEnumType::new("KartID")
        .with_variant("Mario")
        .with_variant("Donkey")
        .with_variant("Yoshi")
        .with_variant("Nokonoko")
        .with_variant("Peach")
        .with_variant("BabyMario")
        .with_variant("Wario")
        .with_variant("Koopa")
        .with_variant("Luigi")
        .with_variant("Diddy")
        .with_variant("Catherine")
        .with_variant("Patapata")
        .with_variant("Daisy")
        .with_variant("BabyLuigi")
        .with_variant("Waluigi")
        .with_variant("KoopaJr")
        .with_variant("Kinopio")
        .with_variant("Kinopico")
        .with_variant("Teresa")
        .with_variant("Pakkun")
        .with_variant("Extra")
}

pub const MAX_VERSION_LENGTH: usize = 19;
pub const MIN_CLIENT_PLAYER_COUNT: usize = 1;
pub const MAX_CLIENT_PLAYER_COUNT: usize = 4;
pub const MAX_CLIENT_KART_COUNT: usize = 4;
pub const KART_CHARACTER_COUNT: usize = 2;
pub const PLAYER_NAME_LENGTH: usize = 3;
pub const MODE_INDEX_COUNT: usize = 5;
pub const MAX_ROOM_KART_COUNT: usize = 8;
pub const MIN_KART_PLAYER_COUNT: usize = 1;
pub const MAX_KART_PLAYER_COUNT: usize = 2;
pub const MAX_TEAM_COUNT: usize = 4;
