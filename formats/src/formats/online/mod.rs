use crate::format::{ConstantList, Format, TypeList};
use crate::formats::online::client_state::*;
use crate::formats::online::mode_index::*;
use crate::formats::online::room_options::*;
use crate::formats::online::server_state::*;
use crate::simple_constant::SimpleConstant;

mod client_state;
mod mode_index;
mod room_options;
mod server_state;

pub fn format() -> Format<impl ConstantList, impl TypeList> {
    let default_port = SimpleConstant::new("DEFAULT_PORT", 3549u16);
    let protocol_version = SimpleConstant::new("PROTOCOL_VERSION", 4u32);
    let max_lap_count = SimpleConstant::new("MAX_LAP_COUNT", 9u8);
    let min_match_count = SimpleConstant::new("MIN_MATCH_COUNT", 1u8);
    let max_match_count = SimpleConstant::new("MAX_MATCH_COUNT", 96u8);
    let default_match_count = SimpleConstant::new("DEFAULT_MATCH_COUNT", 4u8);
    let max_motd_length = SimpleConstant::new("MAX_MOTD_LENGTH", MAX_MOTD_LENGTH);
    Format::new("Online")
        .with_constant(default_port)
        .with_constant(protocol_version)
        .with_constant(max_lap_count)
        .with_constant(min_match_count)
        .with_constant(max_match_count)
        .with_constant(default_match_count)
        .with_constant(max_motd_length)
        .with_type(mode_index())
        .with_type(room_option_code_type())
        .with_type(room_option_format())
        .with_type(room_option_engine_size())
        .with_type(room_option_item_mode())
        .with_type(room_option_course_selection())
        .with_type(room_options_race())
        .with_type(room_options_battle())
        .with_type(client_identity_unspecified())
        .with_type(client_player())
        .with_type(client_identity_specified())
        .with_type(client_identity())
        .with_type(client_state_server())
        .with_type(client_state_mode())
        .with_type(client_state_pack())
        .with_type(client_room_state_new())
        .with_type(client_room_state_code())
        .with_type(client_room_options())
        .with_type(client_room_state_main())
        .with_type(client_room_state())
        .with_type(client_state_room())
        .with_type(client_state())
        .with_type(server_identity_unspecified())
        .with_type(server_identity_specified())
        .with_type(server_identity())
        .with_type(server_state_server())
        .with_type(server_mode())
        .with_type(server_state_mode())
        .with_type(server_state_pack())
        .with_type(server_player())
        .with_type(server_kart())
        .with_type(server_room_options())
        .with_type(server_room_state_main())
        .with_type(server_room_state())
        .with_type(server_state_room())
        .with_type(server_state())
}
