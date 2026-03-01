use crate::formats::online::ServerPlayer;
use crate::mmr::Mmr;

#[derive(Clone, Debug)]
pub struct Player {
    player: ServerPlayer,
    mmr: u16,
}

impl Player {
    pub const fn new(player: ServerPlayer, mmr: u16) -> Self {
        Self { player, mmr }
    }

    pub const fn player(&self) -> &ServerPlayer {
        &self.player
    }
}

impl Mmr for Player {
    fn mmr(&self) -> u16 {
        self.mmr
    }
}
