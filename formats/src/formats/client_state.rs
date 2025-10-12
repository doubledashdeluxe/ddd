use crate::array_type::ArrayType;
use crate::complex_data_type::ComplexDataType;
use crate::enum_type::EnumType;
use crate::format::{ConstantList, Format, TypeList};
use crate::simple_data_type::SimpleDataType;
use crate::struct_type::StructType;

pub fn format() -> Format<impl ConstantList, impl TypeList> {
    Format::new("ClientState")
        .with_type(client_identity_unspecified())
        .with_type(client_player())
        .with_type(client_identity_specified())
        .with_type(client_identity())
        .with_type(client_state_server())
        .with_type(client_state_mode())
        .with_type(client_state_pack())
        .with_type(client_state())
}

fn client_state() -> impl ComplexDataType {
    EnumType::new("ClientState")
        .with_variant("Server", client_state_server())
        .with_variant("Mode", client_state_mode())
        .with_variant("Pack", client_state_pack())
}

fn client_state_server() -> impl ComplexDataType {
    let protocol_version: SimpleDataType<u32> = SimpleDataType::new();
    let version_element: SimpleDataType<u8> = SimpleDataType::new();
    let version = ArrayType::new(version_element, 0, 19);
    StructType::new("ClientStateServer")
        .with_field("protocol_version", protocol_version)
        .with_field("version", version)
        .with_field("client_identity", client_identity())
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
    let players = ArrayType::new(client_player(), 1, 4);
    StructType::new("ClientIdentitySpecified").with_field("players", players)
}

fn client_player() -> impl ComplexDataType {
    let profile: SimpleDataType<u8> = SimpleDataType::new();
    let name_element: SimpleDataType<u8> = SimpleDataType::new();
    let name = ArrayType::new(name_element, 3, 3);
    StructType::new("ClientPlayer").with_field("profile", profile).with_field("name", name)
}

fn client_state_mode() -> impl ComplexDataType {
    StructType::new("ClientStateMode")
}

fn client_state_pack() -> impl ComplexDataType {
    let mode_index: SimpleDataType<u8> = SimpleDataType::new();
    let pack_index: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    StructType::new("ClientStatePack")
        .with_field("mode_index", mode_index)
        .with_field("pack_index", pack_index)
        .with_field("pack_hash", pack_hash)
}
