use crate::formats::online::ServerPlayer;
use crate::mmr::Mmr;

#[derive(Clone, Debug)]
pub struct Player {
    player: ServerPlayer,
    mmr: u16,
}

impl Player {
    pub fn new(player: ServerPlayer, mmr: u16) -> Player {
        Player { player, mmr }
    }

    pub fn player(&self) -> &ServerPlayer {
        &self.player
    }
}

impl Mmr for Player {
    fn mmr(&self) -> u16 {
        self.mmr
    }
}
