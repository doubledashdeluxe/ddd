pub use crate::storage::race::kart::Kart;
pub use crate::storage::race::player::Player;
pub use crate::storage::race::team::Team;
pub use crate::storage::race::version::Version;

use std::cmp::Reverse;

use heapless::Vec;
use jiff::Timestamp;
use serde::{Deserialize, Serialize};

use crate::base64;
use crate::crypto::PublicKey;
use crate::formats::online::*;

mod kart;
mod player;
mod team;
mod version;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Race {
    #[serde(skip)]
    pub number: u64,
    #[serde(skip)]
    pub room_id: u128,
    pub room_number: u64,
    pub frame_rate: FrameRate,
    #[serde(skip)]
    pub host_pk: Option<PublicKey>,
    pub karts: Vec<Kart, MAX_ROOM_KART_COUNT>,
    pub spectator_count: u64,
    pub mode: ModeIndex,
    pub pack_course_count: usize,
    #[serde(with = "base64")]
    pub pack_hash: [u8; 32],
    pub code_type: RoomOptionCodeType,
    pub format: RoomOptionFormat,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub engine_size: Option<RoomOptionEngineSize>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub item_mode: Option<RoomOptionItemMode>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub lap_count: Option<u8>,
    #[serde(rename = "match_count")]
    pub race_count: u8,
    pub course_selection: RoomOptionCourseSelection,
    #[serde(rename = "match_index")]
    pub race_index: u8,
    pub start: Timestamp,
    pub selected_kart_index: u8,
    #[serde(with = "base64")]
    pub course_hash: [u8; 32],
    pub end: Timestamp,
    #[serde(default)]
    pub version: Version,
}

impl Race {
    pub fn teams(&self) -> Vec<Team, MAX_TEAM_COUNT> {
        let team_count = self.karts.iter().map(|kart| kart.team).max().unwrap_or(0);
        let mut teams: Vec<_, _> =
            (0..=team_count).map(|team| Team { team, point_diff: 0, points: 0 }).collect();
        for kart in &self.karts {
            let point_diff = i32::from(kart.result_points) - i32::from(kart.points);
            teams[kart.team as usize].point_diff += point_diff;
            teams[kart.team as usize].points += kart.result_points;
        }
        teams.sort_unstable_by_key(|team| (Reverse(team.point_diff), team.points, team.team));
        teams
    }
}
