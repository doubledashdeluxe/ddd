use heapless::LinearMap;

use crate::crypto::PublicKey;
use crate::formats::online::*;
use crate::room::{Inputs, RaceClient};
use crate::storage::player::Player;
use crate::storage::race::Race;

#[derive(Debug)]
pub struct Batch {
    pub clients: LinearMap<PublicKey, RaceClient, MAX_REPLAY_CLIENT_COUNT>,
    pub inputs: heapless::Vec<Inputs, MAX_ROOM_KART_COUNT>,
    pub room_state: ServerRoomStateMain,
    pub team_state: Option<ServerTeamStateMain>,
    pub poll_state: ServerPollStateReady,
    pub race_states: Vec<ServerRaceStateMain>,
    pub players: heapless::Vec<Player, MAX_ROOM_PLAYER_COUNT>,
    pub race: Race,
}
