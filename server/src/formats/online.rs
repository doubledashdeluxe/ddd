#![allow(clippy::result_unit_err)]
#![allow(clippy::trivially_copy_pass_by_ref)]
#![allow(clippy::unnecessary_wraps)]
#![allow(dead_code)]

use std::array;
use std::fmt::{self, Display, Formatter};

use rand::distr::{Distribution, StandardUniform};
use rand::{Rng, RngExt};

use crate::frequency::Frequency;
use crate::weight::Weight;

include!(concat!(env!("OUT_DIR"), "/online.rs"));

pub type CharacterId = CharacterID;
pub type KartId = KartID;
pub type ItemId = ItemID;

impl Display for FrameRate {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        let frame_rate = match self {
            Self::SixtyHz => "60 Hz",
            Self::FiftyHz => "50 Hz",
        };
        write!(f, "{frame_rate}")
    }
}

impl TryFrom<Frequency> for FrameRate {
    type Error = ();

    fn try_from(frequency: Frequency) -> Result<Self, Self::Error> {
        match frequency {
            Frequency::SixtyHz => Ok(Self::SixtyHz),
            Frequency::FiftyHz => Ok(Self::FiftyHz),
            Frequency::FiveHundredHz => Err(()),
        }
    }
}

impl ModeIndex {
    pub const fn is_race(self) -> bool {
        match self {
            Self::Versus => true,
            Self::Balloon => false,
            Self::Escape => false,
            Self::Bomb => false,
            Self::TimeAttack => true,
        }
    }
}

impl Display for RoomOptionEngineSize {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        let engine_size = match self {
            Self::Small => "50cc",
            Self::Medium => "100cc",
            Self::Large => "150cc",
            Self::Mirror => "Mirror",
        };
        write!(f, "{engine_size}")
    }
}

impl Display for RoomOptionItemMode {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        let item_mode = match self {
            Self::Recommended => "Recommended Items",
            Self::Basic => "Basic Items",
            Self::Frantic => "Frantic Items",
            Self::None => "No Items",
        };
        write!(f, "{item_mode}")
    }
}

impl CharacterId {
    pub const fn weight(self) -> Weight {
        match self {
            Self::BabyMario => Weight::Light,
            Self::BabyLuigi => Weight::Light,
            Self::Patapata => Weight::Light,
            Self::Nokonoko => Weight::Light,
            Self::Peach => Weight::Medium,
            Self::Daisy => Weight::Medium,
            Self::Mario => Weight::Medium,
            Self::Luigi => Weight::Medium,
            Self::Wario => Weight::Heavy,
            Self::Waluigi => Weight::Medium,
            Self::Yoshi => Weight::Medium,
            Self::Catherine => Weight::Medium,
            Self::Donkey => Weight::Heavy,
            Self::Diddy => Weight::Light,
            Self::Koopa => Weight::Heavy,
            Self::KoopaJr => Weight::Light,
            Self::Kinopio => Weight::Light,
            Self::Kinopico => Weight::Light,
            Self::Teresa => Weight::Heavy,
            Self::Pakkun => Weight::Heavy,
        }
    }

    pub const fn special_item(self) -> ItemId {
        match self {
            Self::BabyMario => ItemId::Chomp,
            Self::BabyLuigi => ItemId::Chomp,
            Self::Patapata => ItemId::TripleGreenShells,
            Self::Nokonoko => ItemId::TripleGreenShells,
            Self::Peach => ItemId::Heart,
            Self::Daisy => ItemId::Heart,
            Self::Mario => ItemId::Fireballs,
            Self::Luigi => ItemId::Fireballs,
            Self::Wario => ItemId::Bomb,
            Self::Waluigi => ItemId::Bomb,
            Self::Yoshi => ItemId::YoshiEgg,
            Self::Catherine => ItemId::YoshiEgg,
            Self::Donkey => ItemId::GiantBanana,
            Self::Diddy => ItemId::GiantBanana,
            Self::Koopa => ItemId::BowserShell,
            Self::KoopaJr => ItemId::BowserShell,
            Self::Kinopio => ItemId::GoldenMushroom,
            Self::Kinopico => ItemId::GoldenMushroom,
            Self::Teresa => ItemId::None,
            Self::Pakkun => ItemId::None,
        }
    }
}

impl Distribution<CharacterId> for StandardUniform {
    fn sample<R: Rng + ?Sized>(&self, rng: &mut R) -> CharacterId {
        match rng.random_range(0..=19) {
            0 => CharacterId::BabyMario,
            1 => CharacterId::BabyLuigi,
            2 => CharacterId::Patapata,
            3 => CharacterId::Nokonoko,
            4 => CharacterId::Peach,
            5 => CharacterId::Daisy,
            6 => CharacterId::Mario,
            7 => CharacterId::Luigi,
            8 => CharacterId::Wario,
            9 => CharacterId::Waluigi,
            10 => CharacterId::Yoshi,
            11 => CharacterId::Catherine,
            12 => CharacterId::Donkey,
            13 => CharacterId::Diddy,
            14 => CharacterId::Koopa,
            15 => CharacterId::KoopaJr,
            16 => CharacterId::Kinopio,
            17 => CharacterId::Kinopico,
            18 => CharacterId::Teresa,
            _ => CharacterId::Pakkun,
        }
    }
}

impl KartId {
    pub const fn weight(self) -> Weight {
        match self {
            Self::Mario => Weight::Medium,
            Self::Donkey => Weight::Heavy,
            Self::Yoshi => Weight::Medium,
            Self::Nokonoko => Weight::Light,
            Self::Peach => Weight::Medium,
            Self::BabyMario => Weight::Light,
            Self::Wario => Weight::Heavy,
            Self::Koopa => Weight::Heavy,
            Self::Luigi => Weight::Medium,
            Self::Diddy => Weight::Light,
            Self::Catherine => Weight::Medium,
            Self::Patapata => Weight::Light,
            Self::Daisy => Weight::Medium,
            Self::BabyLuigi => Weight::Light,
            Self::Waluigi => Weight::Medium,
            Self::KoopaJr => Weight::Light,
            Self::Kinopio => Weight::Light,
            Self::Kinopico => Weight::Light,
            Self::Teresa => Weight::Heavy,
            Self::Pakkun => Weight::Heavy,
            Self::Extra => Weight::Heavy,
        }
    }

    pub fn compatible(self, character_ids: &[CharacterId]) -> bool {
        if self == Self::Extra {
            return true;
        }

        let weight = self.weight();
        let character_weights: [_; 2] = array::from_fn(|i| character_ids[i].weight());
        let max_character_weight = character_weights[0].max(character_weights[1]);
        max_character_weight == weight
    }
}

impl Distribution<KartId> for StandardUniform {
    fn sample<R: Rng + ?Sized>(&self, rng: &mut R) -> KartId {
        match rng.random_range(0..=20) {
            0 => KartId::Mario,
            1 => KartId::Donkey,
            2 => KartId::Yoshi,
            3 => KartId::Nokonoko,
            4 => KartId::Peach,
            5 => KartId::BabyMario,
            6 => KartId::Wario,
            7 => KartId::Koopa,
            8 => KartId::Luigi,
            9 => KartId::Diddy,
            10 => KartId::Catherine,
            11 => KartId::Patapata,
            12 => KartId::Daisy,
            13 => KartId::BabyLuigi,
            14 => KartId::Waluigi,
            15 => KartId::KoopaJr,
            16 => KartId::Kinopio,
            17 => KartId::Kinopico,
            18 => KartId::Teresa,
            19 => KartId::Pakkun,
            _ => KartId::Extra,
        }
    }
}

impl ItemId {
    pub const fn is_special(self) -> bool {
        match self {
            Self::GreenShell => false,
            Self::BowserShell => true,
            Self::RedShell => false,
            Self::Banana => false,
            Self::GiantBanana => true,
            Self::Mushroom => false,
            Self::Star => false,
            Self::Chomp => true,
            Self::Bomb => true,
            Self::MarioFireballs => false, // ?
            Self::Lightning => false,
            Self::YoshiEgg => true,
            Self::GoldenMushroom => true,
            Self::BlueShell => false,
            Self::Heart => true,
            Self::FakeItemBox => false,
            Self::None => false,
            Self::TripleGreenShells => true,
            Self::TripleMushrooms => false,
            Self::TripleRedShells => true,
            Self::Bombs => false,
            Self::Fireballs => true,
        }
    }

    pub const fn can_have_two(self) -> bool {
        match self {
            Self::GreenShell => true,
            Self::RedShell => true,
            Self::Banana => true,
            Self::Mushroom => true,
            Self::FakeItemBox => true,
            _ => false,
        }
    }

    #[must_use]
    pub const fn base(self) -> Self {
        match self {
            Self::TripleGreenShells => Self::GreenShell,
            Self::TripleMushrooms => Self::Mushroom,
            Self::TripleRedShells => Self::RedShell,
            Self::Fireballs => Self::MarioFireballs,
            _ => self,
        }
    }

    pub const fn count(self) -> u8 {
        match self {
            Self::TripleGreenShells => 3,
            Self::TripleMushrooms => 3,
            Self::TripleRedShells => 3,
            Self::Fireballs => 5,
            _ => 1,
        }
    }

    pub const fn max_count(self) -> u8 {
        match self {
            Self::GreenShell => 15,
            Self::BowserShell => 4,
            Self::RedShell => 15,
            Self::Banana => 15,
            Self::GiantBanana => 5,
            Self::Mushroom => 11,
            Self::Star => 3,
            Self::Chomp => 1,
            Self::Bomb => 5,
            Self::MarioFireballs => 5,
            Self::Lightning => 1,
            Self::YoshiEgg => 4,
            Self::GoldenMushroom => 2,
            Self::BlueShell => 1,
            Self::Heart => 2,
            Self::FakeItemBox => 9,
            _ => 0,
        }
    }
}

impl ServerRoomOptions {
    pub const fn code_type(&self) -> RoomOptionCodeType {
        match self {
            Self::RaceOptions(options) => options.code_type,
            Self::BattleOptions(options) => options.code_type,
        }
    }

    pub const fn format(&self) -> RoomOptionFormat {
        match self {
            Self::RaceOptions(options) => options.format,
            Self::BattleOptions(options) => options.format,
        }
    }

    pub const fn engine_size(&self) -> Option<RoomOptionEngineSize> {
        match self {
            Self::RaceOptions(options) => Some(options.engine_size),
            Self::BattleOptions(_) => None,
        }
    }

    pub const fn item_mode(&self) -> Option<RoomOptionItemMode> {
        match self {
            Self::RaceOptions(options) => Some(options.item_mode),
            Self::BattleOptions(_) => None,
        }
    }

    pub const fn lap_count(&self) -> Option<u8> {
        match self {
            Self::RaceOptions(options) => Some(options.lap_count),
            Self::BattleOptions(_) => None,
        }
    }

    pub const fn match_count(&self) -> u8 {
        match self {
            Self::RaceOptions(options) => options.match_count,
            Self::BattleOptions(options) => options.match_count,
        }
    }

    pub const fn course_selection(&self) -> RoomOptionCourseSelection {
        match self {
            Self::RaceOptions(options) => options.course_selection,
            Self::BattleOptions(options) => options.course_selection,
        }
    }
}
