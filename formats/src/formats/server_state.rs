use crate::array_type::ArrayType;
use crate::complex_data_type::ComplexDataType;
use crate::enum_type::EnumType;
use crate::format::{ConstantList, Format, TypeList};
use crate::simple_data_type::SimpleDataType;
use crate::struct_type::StructType;

pub fn format() -> Format<impl ConstantList, impl TypeList> {
    Format::new("ServerState")
        .with_type(server_identity_unspecified())
        .with_type(server_identity_specified())
        .with_type(server_identity())
        .with_type(server_state_server())
        .with_type(server_mode())
        .with_type(server_state_mode())
        .with_type(server_state_pack())
        .with_type(server_state())
}

fn server_state() -> impl ComplexDataType {
    EnumType::new("ServerState")
        .with_variant("Server", server_state_server())
        .with_variant("Mode", server_state_mode())
        .with_variant("Pack", server_state_pack())
}

fn server_state_server() -> impl ComplexDataType {
    let protocol_version: SimpleDataType<u32> = SimpleDataType::new();
    let version_element: SimpleDataType<u8> = SimpleDataType::new();
    let version = ArrayType::new(version_element, 0, 19);
    StructType::new("ServerStateServer")
        .with_field("protocol_version", protocol_version)
        .with_field("version", version)
        .with_field("server_identity", server_identity())
}

fn server_identity() -> impl ComplexDataType {
    EnumType::new("ServerIdentity")
        .with_variant("Unspecified", server_identity_unspecified())
        .with_variant("Specified", server_identity_specified())
}

fn server_identity_unspecified() -> impl ComplexDataType {
    StructType::new("ServerIdentityUnspecified")
}

fn server_identity_specified() -> impl ComplexDataType {
    let motd_element: SimpleDataType<u8> = SimpleDataType::new();
    let motd = ArrayType::new(motd_element, 0, 99);
    let player_count: SimpleDataType<u16> = SimpleDataType::new();
    StructType::new("ServerIdentitySpecified")
        .with_field("motd", motd)
        .with_field("player_count", player_count)
}

fn server_state_mode() -> impl ComplexDataType {
    let modes = ArrayType::new(server_mode(), 5, 5);
    StructType::new("ServerStateMode").with_field("modes", modes)
}

fn server_mode() -> impl ComplexDataType {
    let mmr: SimpleDataType<u16> = SimpleDataType::new();
    let mmrs = ArrayType::new(mmr, 1, 4);
    let player_count: SimpleDataType<u16> = SimpleDataType::new();
    StructType::new("ServerMode").with_field("mmrs", mmrs).with_field("player_count", player_count)
}

fn server_state_pack() -> impl ComplexDataType {
    let mode_index: SimpleDataType<u8> = SimpleDataType::new();
    let pack_index: SimpleDataType<u8> = SimpleDataType::new();
    let player_count: SimpleDataType<u16> = SimpleDataType::new();
    let format_player_count: SimpleDataType<u16> = SimpleDataType::new();
    let format_player_counts = ArrayType::new(format_player_count, 3, 3);
    StructType::new("ServerStatePack")
        .with_field("mode_index", mode_index)
        .with_field("pack_index", pack_index)
        .with_field("player_count", player_count)
        .with_field("format_player_counts", format_player_counts)
}
