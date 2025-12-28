use crate::array_type::ArrayType;
use crate::complex_data_type::ComplexDataType;
use crate::enum_type::EnumType;
use crate::formats::online::common::*;
use crate::simple_data_type::SimpleDataType;
use crate::struct_type::StructType;
use crate::unit_type::UnitType;

pub fn server_state() -> impl ComplexDataType {
    EnumType::new("ServerState")
        .with_variant("Server", server_state_server())
        .with_variant("Mode", server_state_mode())
        .with_variant("Pack", server_state_pack())
        .with_variant("Room", server_state_room())
        .with_variant("Team", server_state_team())
        .with_variant("Poll", server_state_poll())
        .with_variant("Race", server_state_race())
}

pub fn server_state_server() -> impl ComplexDataType {
    let protocol_version: SimpleDataType<u32> = SimpleDataType::new();
    let version_element: SimpleDataType<u8> = SimpleDataType::new();
    let version = ArrayType::new(version_element, 0, MAX_VERSION_LENGTH);
    StructType::new("ServerStateServer")
        .with_field("protocol_version", protocol_version)
        .with_field("version", version)
        .with_field("server_identity", server_identity())
}

pub fn server_identity() -> impl ComplexDataType {
    EnumType::new("ServerIdentity")
        .with_variant("Unspecified", server_identity_unspecified())
        .with_variant("Specified", server_identity_specified())
}

pub fn server_identity_unspecified() -> impl ComplexDataType {
    StructType::new("ServerIdentityUnspecified")
}

pub fn server_identity_specified() -> impl ComplexDataType {
    let motd_element: SimpleDataType<u8> = SimpleDataType::new();
    let motd = ArrayType::new(motd_element, 0, MAX_MOTD_LENGTH);
    let player_count: SimpleDataType<u16> = SimpleDataType::new();
    StructType::new("ServerIdentitySpecified")
        .with_field("motd", motd)
        .with_field("player_count", player_count)
}

pub fn server_state_mode() -> impl ComplexDataType {
    let modes = ArrayType::new(server_mode(), MODE_INDEX_COUNT, MODE_INDEX_COUNT);
    StructType::new("ServerStateMode").with_field("modes", modes)
}

pub fn server_mode() -> impl ComplexDataType {
    let mmr: SimpleDataType<u16> = SimpleDataType::new();
    let mmrs = ArrayType::new(mmr, MIN_CLIENT_PLAYER_COUNT, MAX_CLIENT_PLAYER_COUNT);
    let player_count: SimpleDataType<u16> = SimpleDataType::new();
    StructType::new("ServerMode").with_field("mmrs", mmrs).with_field("player_count", player_count)
}

pub fn server_state_pack() -> impl ComplexDataType {
    let pack_index: SimpleDataType<u8> = SimpleDataType::new();
    let player_count: SimpleDataType<u16> = SimpleDataType::new();
    let format_player_count: SimpleDataType<u16> = SimpleDataType::new();
    let format_player_counts = ArrayType::new(format_player_count, FORMAT_COUNT, FORMAT_COUNT);
    StructType::new("ServerStatePack")
        .with_field("mode_index", mode_index())
        .with_field("pack_index", pack_index)
        .with_field("player_count", player_count)
        .with_field("format_player_counts", format_player_counts)
}

pub fn server_state_room() -> impl ComplexDataType {
    StructType::new("ServerStateRoom").with_field("server_room_state", server_room_state())
}

pub fn server_room_state() -> impl ComplexDataType {
    EnumType::new("ServerRoomState")
        .with_variant("Main", server_room_state_main())
        .with_variant("Error", UnitType)
}

pub fn server_room_state_main() -> impl ComplexDataType {
    let karts = ArrayType::new(server_kart(), 0, MAX_ROOM_KART_COUNT);
    let spectator_count: SimpleDataType<u16> = SimpleDataType::new();
    let pack_course_count: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    let room_counter: SimpleDataType<u32> = SimpleDataType::new();
    let room_code: SimpleDataType<u64> = SimpleDataType::new();
    let spectating_counter: SimpleDataType<u32> = SimpleDataType::new();
    let spectating: SimpleDataType<u8> = SimpleDataType::new();
    let continuing: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ServerRoomStateMain")
        .with_field("karts", karts)
        .with_field("spectator_count", spectator_count)
        .with_field("mode_index", mode_index())
        .with_field("pack_course_count", pack_course_count)
        .with_field("pack_hash", pack_hash)
        .with_field("room_counter", room_counter)
        .with_field("room_code", room_code)
        .with_field("spectating_counter", spectating_counter)
        .with_field("spectating", spectating)
        .with_field("options", server_room_options())
        .with_field("continuing", continuing)
}

pub fn server_room_options() -> impl ComplexDataType {
    EnumType::new("ServerRoomOptions")
        .with_variant("RaceOptions", room_options_race())
        .with_variant("BattleOptions", room_options_battle())
}

pub fn server_kart() -> impl ComplexDataType {
    let local: SimpleDataType<u8> = SimpleDataType::new();
    let players = ArrayType::new(server_player(), MIN_KART_PLAYER_COUNT, MAX_KART_PLAYER_COUNT);
    let mmr: SimpleDataType<u16> = SimpleDataType::new();
    StructType::new("ServerKart")
        .with_field("local", local)
        .with_field("players", players)
        .with_field("mmr", mmr)
}

pub fn server_player() -> impl ComplexDataType {
    let index: SimpleDataType<u8> = SimpleDataType::new();
    let name_element: SimpleDataType<u8> = SimpleDataType::new();
    let name = ArrayType::new(name_element, PLAYER_NAME_LENGTH, PLAYER_NAME_LENGTH);
    StructType::new("ServerPlayer").with_field("index", index).with_field("name", name)
}

pub fn server_state_team() -> impl ComplexDataType {
    StructType::new("ServerStateTeam").with_field("server_team_state", server_team_state())
}

pub fn server_team_state() -> impl ComplexDataType {
    EnumType::new("ServerTeamState")
        .with_variant("Main", server_team_state_main())
        .with_variant("Error", UnitType)
}

pub fn server_team_state_main() -> impl ComplexDataType {
    let team: SimpleDataType<u8> = SimpleDataType::new();
    let teams = ArrayType::new(team, 2, MAX_ROOM_KART_COUNT);
    let entry_index: SimpleDataType<u8> = SimpleDataType::new();
    let continuing: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ServerTeamStateMain")
        .with_field("teams", teams)
        .with_field("entry_index", entry_index)
        .with_field("continuing", continuing)
}

pub fn server_state_poll() -> impl ComplexDataType {
    StructType::new("ServerStatePoll").with_field("server_poll_state", server_poll_state())
}

pub fn server_poll_state() -> impl ComplexDataType {
    EnumType::new("ServerPollState")
        .with_variant("Pending", server_poll_state_pending())
        .with_variant("Ready", server_poll_state_ready())
        .with_variant("Error", UnitType)
}

pub fn server_poll_state_pending() -> impl ComplexDataType {
    let kart_index: SimpleDataType<u8> = SimpleDataType::new();
    let kart_indices = ArrayType::new(kart_index, 0, MAX_ROOM_KART_COUNT);
    StructType::new("ServerPollStatePending").with_field("kart_indices", kart_indices)
}

pub fn server_poll_state_ready() -> impl ComplexDataType {
    let karts = ArrayType::new(server_poll_kart(), 2, MAX_ROOM_KART_COUNT);
    let selected_kart_index: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ServerPollStateReady")
        .with_field("karts", karts)
        .with_field("selected_kart_index", selected_kart_index)
}

pub fn server_poll_kart() -> impl ComplexDataType {
    let kart_index: SimpleDataType<u8> = SimpleDataType::new();
    let character_ids = ArrayType::new(character_id(), KART_CHARACTER_COUNT, KART_CHARACTER_COUNT);
    let course_index: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ServerPollKart")
        .with_field("kart_index", kart_index)
        .with_field("character_ids", character_ids)
        .with_field("kart_id", kart_id())
        .with_field("course_index", course_index)
}

pub fn server_state_race() -> impl ComplexDataType {
    StructType::new("ServerStateRace").with_field("server_race_state", server_race_state())
}

pub fn server_race_state() -> impl ComplexDataType {
    EnumType::new("ServerRaceState")
        .with_variant("Main", server_race_state_main())
        .with_variant("Error", UnitType)
}

pub fn server_race_state_main() -> impl ComplexDataType {
    let frame: SimpleDataType<u16> = SimpleDataType::new();
    let client_frame: SimpleDataType<u16> = SimpleDataType::new();
    let kart_flags: SimpleDataType<u8> = SimpleDataType::new();
    let karts = ArrayType::new(server_race_kart(), 0, MAX_ROOM_KART_COUNT);
    StructType::new("ServerRaceStateMain")
        .with_field("frame", frame)
        .with_field("client_frame", client_frame)
        .with_field("kart_flags", kart_flags)
        .with_field("karts", karts)
}

pub fn server_race_kart() -> impl ComplexDataType {
    let kart_frame: SimpleDataType<u16> = SimpleDataType::new();
    let pos_x: SimpleDataType<i16> = SimpleDataType::new();
    let pos_y: SimpleDataType<i16> = SimpleDataType::new();
    let pos_z: SimpleDataType<i16> = SimpleDataType::new();
    let angle: SimpleDataType<i8> = SimpleDataType::new();
    let vel_x: SimpleDataType<i16> = SimpleDataType::new();
    let vel_y: SimpleDataType<i16> = SimpleDataType::new();
    let vel_z: SimpleDataType<i16> = SimpleDataType::new();
    StructType::new("ServerRaceKart")
        .with_field("kart_frame", kart_frame)
        .with_field("pos_x", pos_x)
        .with_field("pos_y", pos_y)
        .with_field("pos_z", pos_z)
        .with_field("angle", angle)
        .with_field("vel_x", vel_x)
        .with_field("vel_y", vel_y)
        .with_field("vel_z", vel_z)
}

pub const MAX_MOTD_LENGTH: u8 = 99;
pub const FORMAT_COUNT: u8 = 3;
pub const MIN_KART_PLAYER_COUNT: u8 = 1;
pub const MAX_KART_PLAYER_COUNT: u8 = 2;
