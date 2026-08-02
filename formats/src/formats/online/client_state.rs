use crate::array_type::ArrayType;
use crate::complex_data_type::ComplexDataType;
use crate::enum_type::EnumType;
use crate::formats::online::common::*;
use crate::simple_data_type::SimpleDataType;
use crate::struct_type::StructType;
use crate::unit_type::UnitType;

pub fn client_state() -> impl ComplexDataType {
    EnumType::new("ClientState")
        .with_variant("Server", client_state_server()) // Compatible
        .with_variant("Update", client_state_update()) // Compatible
        .with_variant("Mode", client_state_mode())
        .with_variant("Pack", client_state_pack())
        .with_variant("Room", client_state_room())
        .with_variant("Team", client_state_team())
        .with_variant("Poll", client_state_poll())
        .with_variant("Race", client_state_race())
}

pub fn client_state_server() -> impl ComplexDataType {
    let update_version: SimpleDataType<u8> = SimpleDataType::new();
    let reserved: SimpleDataType<u8> = SimpleDataType::new();
    let protocol_version: SimpleDataType<u16> = SimpleDataType::new();
    let version_element: SimpleDataType<u8> = SimpleDataType::new();
    let version = ArrayType::new(version_element, 0, MAX_VERSION_LENGTH);
    StructType::new("ClientStateServer") // Compatible
        .with_field("update_version", update_version)
        .with_field("reserved", reserved)
        .with_field("protocol_version", protocol_version)
        .with_field("version", version)
        .with_field("client_identity", client_identity())
}

pub fn client_identity() -> impl ComplexDataType {
    EnumType::new("ClientIdentity")
        .with_variant("Unspecified", client_identity_unspecified()) // Compatible
        .with_variant("Specified", client_identity_specified())
}

pub fn client_identity_unspecified() -> impl ComplexDataType {
    StructType::new("ClientIdentityUnspecified") // Compatible
}

pub fn client_identity_specified() -> impl ComplexDataType {
    let region: SimpleDataType<u8> = SimpleDataType::new();
    let platform_element: SimpleDataType<u8> = SimpleDataType::new();
    let platform = ArrayType::new(platform_element, 0, MAX_PLATFORM_LENGTH);
    let players = ArrayType::new(client_player(), MIN_CLIENT_PLAYER_COUNT, MAX_CLIENT_PLAYER_COUNT);
    let kart_count: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ClientIdentitySpecified")
        .with_field("frame_rate", frame_rate())
        .with_field("region", region)
        .with_field("platform", platform)
        .with_field("players", players)
        .with_field("kart_count", kart_count)
}

pub fn client_player() -> impl ComplexDataType {
    let profile: SimpleDataType<u8> = SimpleDataType::new();
    let name_element: SimpleDataType<u8> = SimpleDataType::new();
    let name = ArrayType::new(name_element, PLAYER_NAME_LENGTH, PLAYER_NAME_LENGTH);
    StructType::new("ClientPlayer").with_field("profile", profile).with_field("name", name)
}

pub fn client_state_update() -> impl ComplexDataType {
    StructType::new("ClientStateUpdate") // Compatible
        .with_field("client_update_state", client_update_state())
}

pub fn client_update_state() -> impl ComplexDataType {
    EnumType::new("ClientUpdateState")
        .with_variant("Info", client_update_state_info())
        .with_variant("Data", client_update_state_data())
}

pub fn client_update_state_info() -> impl ComplexDataType {
    let region: SimpleDataType<u8> = SimpleDataType::new();
    let platform_element: SimpleDataType<u8> = SimpleDataType::new();
    let platform = ArrayType::new(platform_element, 0, MAX_PLATFORM_LENGTH);
    let language: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ClientUpdateStateInfo")
        .with_field("region", region)
        .with_field("platform", platform)
        .with_field("language", language)
}

pub fn client_update_state_data() -> impl ComplexDataType {
    let index: SimpleDataType<u16> = SimpleDataType::new();
    let indices = ArrayType::new(index, 0, MAX_UPDATE_INDEX_COUNT);
    StructType::new("ClientUpdateStateData").with_field("indices", indices)
}

pub fn client_state_mode() -> impl ComplexDataType {
    StructType::new("ClientStateMode")
}

pub fn client_state_pack() -> impl ComplexDataType {
    let pack_index: SimpleDataType<u8> = SimpleDataType::new();
    let pack_course_count: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    StructType::new("ClientStatePack")
        .with_field("mode_index", mode_index())
        .with_field("pack_index", pack_index)
        .with_field("pack_course_count", pack_course_count)
        .with_field("pack_hash", pack_hash)
}

pub fn client_state_room() -> impl ComplexDataType {
    StructType::new("ClientStateRoom").with_field("client_room_state", client_room_state())
}

pub fn client_room_state() -> impl ComplexDataType {
    EnumType::new("ClientRoomState")
        .with_variant("Search", client_room_state_search())
        .with_variant("New", client_room_state_new())
        .with_variant("Code", client_room_state_code())
        .with_variant("Main", client_room_state_main())
}

pub fn client_room_state_search() -> impl ComplexDataType {
    let pack_course_count: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    let room_counter: SimpleDataType<u32> = SimpleDataType::new();
    StructType::new("ClientRoomStateSearch")
        .with_field("mode_index", mode_index())
        .with_field("pack_course_count", pack_course_count)
        .with_field("pack_hash", pack_hash)
        .with_field("format", room_option_format())
        .with_field("room_counter", room_counter)
}

pub fn client_room_state_new() -> impl ComplexDataType {
    let pack_course_count: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    let room_counter: SimpleDataType<u32> = SimpleDataType::new();
    StructType::new("ClientRoomStateNew")
        .with_field("mode_index", mode_index())
        .with_field("pack_course_count", pack_course_count)
        .with_field("pack_hash", pack_hash)
        .with_field("room_counter", room_counter)
}

pub fn client_room_state_code() -> impl ComplexDataType {
    let room_counter: SimpleDataType<u32> = SimpleDataType::new();
    let room_code: SimpleDataType<u64> = SimpleDataType::new();
    StructType::new("ClientRoomStateCode")
        .with_field("room_counter", room_counter)
        .with_field("room_code", room_code)
}

pub fn client_room_state_main() -> impl ComplexDataType {
    let spectating_counter: SimpleDataType<u32> = SimpleDataType::new();
    let spectating: SimpleDataType<u8> = SimpleDataType::new();
    let continuing: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ClientRoomStateMain")
        .with_field("spectating_counter", spectating_counter)
        .with_field("spectating", spectating)
        .with_field("options", client_room_options())
        .with_field("continuing", continuing)
}

pub fn client_room_options() -> impl ComplexDataType {
    EnumType::new("ClientRoomOptions")
        .with_variant("RaceOptions", room_options_race())
        .with_variant("BattleOptions", room_options_battle())
        .with_variant("None", UnitType)
}

pub fn client_state_team() -> impl ComplexDataType {
    StructType::new("ClientStateTeam").with_field("client_team_state", client_team_state())
}

pub fn client_team_state() -> impl ComplexDataType {
    EnumType::new("ClientTeamState")
        .with_variant("Host", client_team_state_host())
        .with_variant("Guest", client_team_state_guest())
}

pub fn client_team_state_host() -> impl ComplexDataType {
    let team: SimpleDataType<u8> = SimpleDataType::new();
    let teams = ArrayType::new(team, 2, MAX_ROOM_KART_COUNT);
    let entry_index: SimpleDataType<u8> = SimpleDataType::new();
    let continuing: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ClientTeamStateHost")
        .with_field("teams", teams)
        .with_field("entry_index", entry_index)
        .with_field("continuing", continuing)
}

pub fn client_team_state_guest() -> impl ComplexDataType {
    StructType::new("ClientTeamStateGuest")
}

pub fn client_state_poll() -> impl ComplexDataType {
    StructType::new("ClientStatePoll").with_field("client_poll_state", client_poll_state())
}

pub fn client_poll_state() -> impl ComplexDataType {
    EnumType::new("ClientPollState")
        .with_variant("Pending", UnitType)
        .with_variant("Ready", client_poll_state_ready())
}

pub fn client_poll_state_ready() -> impl ComplexDataType {
    let karts = ArrayType::new(client_poll_kart(), 0, MAX_CLIENT_KART_COUNT);
    StructType::new("ClientPollStateReady")
        .with_field("karts", karts)
        .with_field("course_index", client_course_index())
}

pub fn client_poll_kart() -> impl ComplexDataType {
    let character_ids = ArrayType::new(character_id(), KART_CHARACTER_COUNT, KART_CHARACTER_COUNT);
    StructType::new("ClientPollKart")
        .with_field("character_ids", character_ids)
        .with_field("kart_id", kart_id())
}

pub fn client_course_index() -> impl ComplexDataType {
    let specified: SimpleDataType<u8> = SimpleDataType::new();
    EnumType::new("ClientCourseIndex")
        .with_variant("Unspecified", UnitType)
        .with_variant("Specified", specified)
}

pub fn client_state_race() -> impl ComplexDataType {
    let frame: SimpleDataType<u16> = SimpleDataType::new();
    let karts = ArrayType::new(client_race_kart(), 0, MAX_CLIENT_KART_COUNT);
    let item_count: SimpleDataType<u8> = SimpleDataType::new();
    let item_counts = ArrayType::new(item_count, 16, 16);
    let latency: SimpleDataType<u16> = SimpleDataType::new();
    let delayed_frames: SimpleDataType<u32> = SimpleDataType::new();
    StructType::new("ClientStateRace")
        .with_field("frame", frame)
        .with_field("karts", karts)
        .with_field("item_counts", item_counts)
        .with_field("latency", latency)
        .with_field("delayed_frames", delayed_frames)
}

pub fn client_race_kart() -> impl ComplexDataType {
    let input: SimpleDataType<u16> = SimpleDataType::new();
    let inputs = ArrayType::new(input, 0, MAX_KART_INPUT_COUNT);
    let inputs = ArrayType::new(inputs, MIN_KART_PLAYER_COUNT, MAX_KART_PLAYER_COUNT);
    let driver: SimpleDataType<u8> = SimpleDataType::new();
    let pos_x: SimpleDataType<i16> = SimpleDataType::new();
    let pos_y: SimpleDataType<i16> = SimpleDataType::new();
    let pos_z: SimpleDataType<i16> = SimpleDataType::new();
    let angle: SimpleDataType<i8> = SimpleDataType::new();
    let vel_x: SimpleDataType<i16> = SimpleDataType::new();
    let vel_z: SimpleDataType<i16> = SimpleDataType::new();
    let item_frame: SimpleDataType<u16> = SimpleDataType::new();
    let item_frames = ArrayType::new(item_frame, KART_CHARACTER_COUNT, KART_CHARACTER_COUNT);
    let item_event_counter: SimpleDataType<u8> = SimpleDataType::new();
    let item_events = ArrayType::new(item_event(), 0, MAX_ITEM_EVENT_COUNT);
    let rank: SimpleDataType<u8> = SimpleDataType::new();
    let lap: SimpleDataType<u8> = SimpleDataType::new();
    let time: SimpleDataType<u32> = SimpleDataType::new();
    StructType::new("ClientRaceKart")
        .with_field("inputs", inputs)
        .with_field("driver", driver)
        .with_field("pos_x", pos_x)
        .with_field("pos_y", pos_y)
        .with_field("pos_z", pos_z)
        .with_field("angle", angle)
        .with_field("vel_x", vel_x)
        .with_field("vel_z", vel_z)
        .with_field("item_frames", item_frames)
        .with_field("item_event_counter", item_event_counter)
        .with_field("item_events", item_events)
        .with_field("rank", rank)
        .with_field("lap", lap)
        .with_field("time", time)
}

pub const MAX_PLATFORM_LENGTH: usize = 31;
pub const MAX_UPDATE_INDEX_COUNT: usize = 8;
pub const MAX_KART_INPUT_COUNT: usize = 30;
