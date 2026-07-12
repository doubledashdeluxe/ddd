use crate::format::Format;
use crate::formats::online::client_state::*;
use crate::formats::online::common::*;
use crate::formats::online::server_state::*;
use crate::simple_constant::SimpleConstant;

mod client_state;
mod common;
mod server_state;

pub fn format() -> Format {
    let default_port = SimpleConstant::new("DEFAULT_PORT", 3549u16);
    let buffer_size = SimpleConstant::new("BUFFER_SIZE", 1152u16);
    let update_version = SimpleConstant::new("UPDATE_VERSION", 1u8);
    let protocol_version = SimpleConstant::new("PROTOCOL_VERSION", 19u16);
    let max_lap_count = SimpleConstant::new("MAX_LAP_COUNT", 9u8);
    let min_match_count = SimpleConstant::new("MIN_MATCH_COUNT", 1u8);
    let max_match_count = SimpleConstant::new("MAX_MATCH_COUNT", 96u8);
    let default_match_count = SimpleConstant::new("DEFAULT_MATCH_COUNT", 4u8);
    let max_version_length = SimpleConstant::new("MAX_VERSION_LENGTH", MAX_VERSION_LENGTH);
    let max_platform_length = SimpleConstant::new("MAX_PLATFORM_LENGTH", MAX_PLATFORM_LENGTH);
    let min_client_player_count =
        SimpleConstant::new("MIN_CLIENT_PLAYER_COUNT", MIN_CLIENT_PLAYER_COUNT);
    let max_client_player_count =
        SimpleConstant::new("MAX_CLIENT_PLAYER_COUNT", MAX_CLIENT_PLAYER_COUNT);
    let max_client_kart_count = SimpleConstant::new("MAX_CLIENT_KART_COUNT", MAX_CLIENT_KART_COUNT);
    let kart_character_count = SimpleConstant::new("KART_CHARACTER_COUNT", KART_CHARACTER_COUNT);
    let player_name_length = SimpleConstant::new("PLAYER_NAME_LENGTH", PLAYER_NAME_LENGTH);
    let mode_index_count = SimpleConstant::new("MODE_INDEX_COUNT", MODE_INDEX_COUNT);
    let max_motd_length = SimpleConstant::new("MAX_MOTD_LENGTH", MAX_MOTD_LENGTH);
    let max_update_size = SimpleConstant::new("MAX_UPDATE_SIZE", 256 * 256 * 1024u32);
    let max_update_changelog_length =
        SimpleConstant::new("MAX_UPDATE_CHANGELOG_LENGTH", MAX_UPDATE_CHANGELOG_LENGTH);
    let max_update_index_count =
        SimpleConstant::new("MAX_UPDATE_INDEX_COUNT", MAX_UPDATE_INDEX_COUNT);
    let update_chunk_size = SimpleConstant::new("UPDATE_CHUNK_SIZE", UPDATE_CHUNK_SIZE);
    let max_team_count = SimpleConstant::new("MAX_TEAM_COUNT", MAX_TEAM_COUNT);
    let format_count = SimpleConstant::new("FORMAT_COUNT", FORMAT_COUNT);
    let max_room_kart_count = SimpleConstant::new("MAX_ROOM_KART_COUNT", MAX_ROOM_KART_COUNT);
    let min_kart_player_count = SimpleConstant::new("MIN_KART_PLAYER_COUNT", MIN_KART_PLAYER_COUNT);
    let max_kart_player_count = SimpleConstant::new("MAX_KART_PLAYER_COUNT", MAX_KART_PLAYER_COUNT);
    let min_client_frame = SimpleConstant::new("MIN_CLIENT_FRAME", 360u16);
    let max_kart_input_count = SimpleConstant::new("MAX_KART_INPUT_COUNT", MAX_KART_INPUT_COUNT);
    let max_item_event_count = SimpleConstant::new("MAX_ITEM_EVENT_COUNT", MAX_ITEM_EVENT_COUNT);
    let min_stick_y = SimpleConstant::new("MIN_STICK_Y", -3i8);
    let max_stick_y = SimpleConstant::new("MAX_STICK_Y", 3i8);
    Format::new("Online")
        .with_constant(default_port)
        .with_constant(buffer_size)
        .with_constant(update_version)
        .with_constant(protocol_version)
        .with_constant(max_lap_count)
        .with_constant(min_match_count)
        .with_constant(max_match_count)
        .with_constant(default_match_count)
        .with_constant(max_version_length)
        .with_constant(max_platform_length)
        .with_constant(min_client_player_count)
        .with_constant(max_client_player_count)
        .with_constant(max_client_kart_count)
        .with_constant(kart_character_count)
        .with_constant(player_name_length)
        .with_constant(mode_index_count)
        .with_constant(max_motd_length)
        .with_constant(max_update_size)
        .with_constant(max_update_changelog_length)
        .with_constant(max_update_index_count)
        .with_constant(update_chunk_size)
        .with_constant(max_team_count)
        .with_constant(format_count)
        .with_constant(max_room_kart_count)
        .with_constant(min_kart_player_count)
        .with_constant(max_kart_player_count)
        .with_constant(min_client_frame)
        .with_constant(max_kart_input_count)
        .with_constant(max_item_event_count)
        .with_constant(min_stick_y)
        .with_constant(max_stick_y)
        .with_type(frame_rate())
        .with_type(mode_index())
        .with_type(room_option_code_type())
        .with_type(room_option_format())
        .with_type(room_option_engine_size())
        .with_type(room_option_item_mode())
        .with_type(room_option_course_selection())
        .with_type(room_options_race())
        .with_type(room_options_battle())
        .with_type(character_id())
        .with_type(kart_id())
        .with_type(item_id())
        .with_type(item_event())
        .with_type(client_identity_unspecified())
        .with_type(client_player())
        .with_type(client_identity_specified())
        .with_type(client_identity())
        .with_type(client_state_server())
        .with_type(client_update_state_info())
        .with_type(client_update_state_data())
        .with_type(client_update_state())
        .with_type(client_state_update())
        .with_type(client_state_mode())
        .with_type(client_state_pack())
        .with_type(client_room_state_search())
        .with_type(client_room_state_new())
        .with_type(client_room_state_code())
        .with_type(client_room_options())
        .with_type(client_room_state_main())
        .with_type(client_room_state())
        .with_type(client_state_room())
        .with_type(client_team_state_host())
        .with_type(client_team_state_guest())
        .with_type(client_team_state())
        .with_type(client_state_team())
        .with_type(client_poll_kart())
        .with_type(client_course_index())
        .with_type(client_poll_state_ready())
        .with_type(client_poll_state())
        .with_type(client_state_poll())
        .with_type(client_race_kart())
        .with_type(client_state_race())
        .with_type(client_state())
        .with_type(server_identity_unspecified())
        .with_type(server_identity_specified())
        .with_type(server_identity())
        .with_type(server_state_server())
        .with_type(server_update_state_info())
        .with_type(server_update_state_data())
        .with_type(server_update_state())
        .with_type(server_state_update())
        .with_type(server_mode())
        .with_type(server_state_mode())
        .with_type(server_state_pack())
        .with_type(server_player())
        .with_type(server_kart())
        .with_type(server_room_options())
        .with_type(server_room_state_main())
        .with_type(server_room_state())
        .with_type(server_state_room())
        .with_type(server_team_state_main())
        .with_type(server_team_state())
        .with_type(server_state_team())
        .with_type(server_poll_state_pending())
        .with_type(server_poll_kart())
        .with_type(server_poll_state_ready())
        .with_type(server_poll_state())
        .with_type(server_state_poll())
        .with_type(server_race_kart())
        .with_type(server_race_state_main())
        .with_type(server_race_state())
        .with_type(server_state_race())
        .with_type(server_state())
}
