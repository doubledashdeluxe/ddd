use std::fmt::{Display, Formatter, Result};

use crate::formats::online::{CharacterId, KartId};

#[derive(Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct Combo {
    characters: [CharacterId; 2],
    kart: KartId,
}

impl Combo {
    pub fn new(mut characters: [CharacterId; 2], kart: KartId) -> Self {
        if characters[0] > characters[1] {
            characters.swap(0, 1);
        }
        Self { characters, kart }
    }
}

impl Display for Combo {
    fn fmt(&self, f: &mut Formatter) -> Result {
        write!(f, "{} + {} + {}", self.characters[0], self.characters[1], self.kart)
    }
}
