use crate::array_type::ArrayType;
use crate::complex_data_type::ComplexDataType;
use crate::enum_type::EnumType;
use crate::formats::online::common::*;
use crate::simple_data_type::SimpleDataType;
use crate::struct_type::StructType;
use crate::unit_type::UnitType;

pub fn client_state() -> impl ComplexDataType {
    EnumType::new("ClientState")
        .with_variant("Server", client_state_server())
        .with_variant("Mode", client_state_mode())
        .with_variant("Pack", client_state_pack())
        .with_variant("Room", client_state_room())
}

pub fn client_state_server() -> impl ComplexDataType {
    let protocol_version: SimpleDataType<u32> = SimpleDataType::new();
    let version_element: SimpleDataType<u8> = SimpleDataType::new();
    let version = ArrayType::new(version_element, 0, MAX_VERSION_LENGTH);
    StructType::new("ClientStateServer")
        .with_field("protocol_version", protocol_version)
        .with_field("version", version)
        .with_field("client_identity", client_identity())
}

pub fn client_identity() -> impl ComplexDataType {
    EnumType::new("ClientIdentity")
        .with_variant("Unspecified", client_identity_unspecified())
        .with_variant("Specified", client_identity_specified())
}

pub fn client_identity_unspecified() -> impl ComplexDataType {
    StructType::new("ClientIdentityUnspecified")
}

pub fn client_identity_specified() -> impl ComplexDataType {
    let players = ArrayType::new(client_player(), 1, 4);
    let kart_count: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ClientIdentitySpecified")
        .with_field("players", players)
        .with_field("kart_count", kart_count)
}

pub fn client_player() -> impl ComplexDataType {
    let profile: SimpleDataType<u8> = SimpleDataType::new();
    let name_element: SimpleDataType<u8> = SimpleDataType::new();
    let name = ArrayType::new(name_element, 3, 3);
    StructType::new("ClientPlayer").with_field("profile", profile).with_field("name", name)
}

pub fn client_state_mode() -> impl ComplexDataType {
    StructType::new("ClientStateMode")
}

pub fn client_state_pack() -> impl ComplexDataType {
    let pack_index: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    StructType::new("ClientStatePack")
        .with_field("mode_index", mode_index())
        .with_field("pack_index", pack_index)
        .with_field("pack_hash", pack_hash)
}

pub fn client_state_room() -> impl ComplexDataType {
    StructType::new("ClientStateRoom").with_field("client_room_state", client_room_state())
}

pub fn client_room_state() -> impl ComplexDataType {
    EnumType::new("ClientRoomState")
        .with_variant("New", client_room_state_new())
        .with_variant("Code", client_room_state_code())
        .with_variant("Main", client_room_state_main())
}

pub fn client_room_state_new() -> impl ComplexDataType {
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    let room_counter: SimpleDataType<u32> = SimpleDataType::new();
    StructType::new("ClientRoomStateNew")
        .with_field("mode_index", mode_index())
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
    StructType::new("ClientRoomStateMain")
        .with_field("spectating_counter", spectating_counter)
        .with_field("spectating", spectating)
        .with_field("options", client_room_options())
}

pub fn client_room_options() -> impl ComplexDataType {
    EnumType::new("ClientRoomOptions")
        .with_variant("Race", room_options_race())
        .with_variant("Battle", room_options_battle())
        .with_variant("None", UnitType)
}
