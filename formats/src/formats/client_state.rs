use crate::array_type::ArrayType;
use crate::complex_data_type::ComplexDataType;
use crate::enum_type::EnumType;
use crate::format::{ConstantList, Format, TypeList};
use crate::simple_data_type::SimpleDataType;
use crate::struct_type::StructType;

pub fn format() -> Format<impl ConstantList, impl TypeList> {
    Format::new("ClientState")
        .with_type(client_version())
        .with_type(client_identity_unspecified())
        .with_type(client_identity_specified())
        .with_type(client_identity())
        .with_type(client_state_server())
        .with_type(client_state_room())
        .with_type(client_state())
}

fn client_state() -> impl ComplexDataType {
    EnumType::new("ClientState")
        .with_variant("Server", client_state_server())
        .with_variant("Room", client_state_room())
}

fn client_state_server() -> impl ComplexDataType {
    let protocol_version: SimpleDataType<u32> = SimpleDataType::new();
    StructType::new("ClientStateServer")
        .with_field("protocol_version", protocol_version)
        .with_field("client_version", client_version())
        .with_field("client_identity", client_identity())
}

fn client_version() -> impl ComplexDataType {
    let major: SimpleDataType<u8> = SimpleDataType::new();
    let minor: SimpleDataType<u8> = SimpleDataType::new();
    let patch: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ClientVersion")
        .with_field("major", major)
        .with_field("minor", minor)
        .with_field("patch", patch)
}

fn client_identity() -> impl ComplexDataType {
    EnumType::new("ClientIdentity")
        .with_variant("Unspecified", client_identity_unspecified())
        .with_variant("Specified", client_identity_specified())
}

fn client_identity_unspecified() -> impl ComplexDataType {
    StructType::new("ClientIdentityUnspecified")
}

fn client_identity_specified() -> impl ComplexDataType {
    let player_element: SimpleDataType<u8> = SimpleDataType::new();
    let player = ArrayType::new(player_element, 1, 3);
    let players = ArrayType::new(player, 1, 2);
    let players = ArrayType::new(players, 1, 4);
    StructType::new("ClientIdentitySpecified").with_field("players", players)
}

fn client_state_room() -> impl ComplexDataType {
    StructType::new("ClientStateRoom")
}
