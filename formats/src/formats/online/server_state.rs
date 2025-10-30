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
    let format_player_counts = ArrayType::new(format_player_count, 3, 3);
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
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    let room_counter: SimpleDataType<u32> = SimpleDataType::new();
    let room_code: SimpleDataType<u64> = SimpleDataType::new();
    let spectating_counter: SimpleDataType<u32> = SimpleDataType::new();
    let spectating: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ServerRoomStateMain")
        .with_field("karts", karts)
        .with_field("spectator_count", spectator_count)
        .with_field("mode_index", mode_index())
        .with_field("pack_hash", pack_hash)
        .with_field("room_counter", room_counter)
        .with_field("room_code", room_code)
        .with_field("spectating_counter", spectating_counter)
        .with_field("spectating", spectating)
        .with_field("options", server_room_options())
}

pub fn server_room_options() -> impl ComplexDataType {
    EnumType::new("ServerRoomOptions")
        .with_variant("Race", room_options_race())
        .with_variant("Battle", room_options_battle())
}

pub fn server_kart() -> impl ComplexDataType {
    let players = ArrayType::new(server_player(), MIN_KART_PLAYER_COUNT, MAX_KART_PLAYER_COUNT);
    StructType::new("ServerKart").with_field("players", players)
}

pub fn server_player() -> impl ComplexDataType {
    let name_element: SimpleDataType<u8> = SimpleDataType::new();
    let name = ArrayType::new(name_element, PLAYER_NAME_LENGTH, PLAYER_NAME_LENGTH);
    StructType::new("ServerPlayer").with_field("name", name)
}

pub const MAX_MOTD_LENGTH: u8 = 99;
pub const MAX_ROOM_KART_COUNT: u8 = 8;
pub const MIN_KART_PLAYER_COUNT: u8 = 1;
pub const MAX_KART_PLAYER_COUNT: u8 = 2;
