use crate::array_type::ArrayType;
use crate::complex_data_type::ComplexDataType;
use crate::formats::online::client_state::{self, *};
use crate::formats::online::common::*;
use crate::simple_data_type::SimpleDataType;
use crate::struct_type::StructType;

pub fn replay() -> impl ComplexDataType {
    let magic: SimpleDataType<u32> = SimpleDataType::new();
    let replay_version: SimpleDataType<u16> = SimpleDataType::new();
    let reserved: SimpleDataType<u16> = SimpleDataType::new();
    let version_element: SimpleDataType<u8> = SimpleDataType::new();
    let version = ArrayType::new(version_element, 0, MAX_VERSION_LENGTH);
    StructType::new("Replay") // Compatible
        .with_field("magic", magic)
        .with_field("replay_version", replay_version)
        .with_field("reserved", reserved)
        .with_field("version", version)
}

pub fn replay_race() -> impl ComplexDataType {
    let pack_course_count: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    let course_index: SimpleDataType<u8> = SimpleDataType::new();
    let clients = ArrayType::new(replay_client(), 1, MAX_REPLAY_CLIENT_COUNT);
    let time: SimpleDataType<u64> = SimpleDataType::new();
    let room_code: SimpleDataType<u64> = SimpleDataType::new();
    StructType::new("ReplayRace")
        .with_field("frame_rate", frame_rate())
        .with_field("mode_index", mode_index())
        .with_field("pack_course_count", pack_course_count)
        .with_field("pack_hash", pack_hash)
        .with_field("course_index", course_index)
        .with_field("clients", clients)
        .with_field("time", time)
        .with_field("room_type", room_type())
        .with_field("format", room_option_format())
        .with_field("room_code", room_code)
}

pub fn replay_client() -> impl ComplexDataType {
    let pk_element: SimpleDataType<u8> = SimpleDataType::new();
    let pk = ArrayType::new(pk_element, 32, 32);
    let region: SimpleDataType<u8> = SimpleDataType::new();
    let platform_element: SimpleDataType<u8> = SimpleDataType::new();
    let platform = ArrayType::new(platform_element, 0, MAX_PLATFORM_LENGTH);
    let players = ArrayType::new(
        client_state::client_player(),
        MIN_CLIENT_PLAYER_COUNT,
        MAX_CLIENT_PLAYER_COUNT,
    );
    let team: SimpleDataType<u8> = SimpleDataType::new();
    let teams = ArrayType::new(team, MIN_CLIENT_KART_COUNT, MAX_CLIENT_KART_COUNT);
    StructType::new("ReplayClient")
        .with_field("pk", pk)
        .with_field("region", region)
        .with_field("platform", platform)
        .with_field("players", players)
        .with_field("teams", teams)
}

pub fn replay_state() -> impl ComplexDataType {
    let client_states = ArrayType::new(replay_client_state(), 0, MAX_KART_INPUT_COUNT);
    let client_states = ArrayType::new(client_states, 0, MAX_REPLAY_CLIENT_COUNT);
    StructType::new("ReplayState").with_field("client_states", client_states)
}

pub fn replay_client_state() -> impl ComplexDataType {
    let replay_server_frame: SimpleDataType<u16> = SimpleDataType::new();
    let replay_client_frame: SimpleDataType<u16> = SimpleDataType::new();
    let replay_input: SimpleDataType<u16> = SimpleDataType::new();
    let replay_inputs =
        ArrayType::new(replay_input, MIN_CLIENT_PLAYER_COUNT, MAX_CLIENT_PLAYER_COUNT);
    StructType::new("ReplayClientState")
        .with_field("replay_server_frame", replay_server_frame)
        .with_field("replay_client_frame", replay_client_frame)
        .with_field("replay_inputs", replay_inputs)
}

pub const MAX_REPLAY_CLIENT_COUNT: usize = 8;
