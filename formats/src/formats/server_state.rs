use crate::complex_data_type::ComplexDataType;
use crate::enum_type::EnumType;
use crate::format::{Format, TypeList};
use crate::simple_data_type::SimpleDataType;
use crate::struct_type::StructType;

pub fn format() -> Format<impl TypeList> {
    Format::new("ServerState")
        .with_type(server_version())
        .with_type(server_identity_unspecified())
        .with_type(server_identity_specified())
        .with_type(server_identity())
        .with_type(server_state_server())
        .with_type(server_state_room())
        .with_type(server_state())
}

fn server_state() -> impl ComplexDataType {
    EnumType::new("ServerState")
        .with_variant("Server", server_state_server())
        .with_variant("Room", server_state_room())
}

fn server_state_server() -> impl ComplexDataType {
    let protocol_version: SimpleDataType<u32> = SimpleDataType::new();
    StructType::new("ServerStateServer")
        .with_field("protocol_version", protocol_version)
        .with_field("server_version", server_version())
        .with_field("server_identity", server_identity())
}

fn server_version() -> impl ComplexDataType {
    let major: SimpleDataType<u8> = SimpleDataType::new();
    let minor: SimpleDataType<u8> = SimpleDataType::new();
    let patch: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ServerVersion")
        .with_field("major", major)
        .with_field("minor", minor)
        .with_field("patch", patch)
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
    StructType::new("ServerIdentitySpecified")
}

fn server_state_room() -> impl ComplexDataType {
    StructType::new("ServerStateRoom")
}
