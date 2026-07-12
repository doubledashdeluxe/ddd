use std::time::Duration;

use crate::formats::online::FrameRate;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[expect(clippy::enum_variant_names)]
pub enum Frequency {
    SixtyHz,
    FiftyHz,
    FiveHundredHz,
}

impl Frequency {
    pub const fn period(self) -> Duration {
        let duration = match self {
            Self::SixtyHz => 16_683_333,
            Self::FiftyHz => 20_000_000,
            Self::FiveHundredHz => 2_000_000,
        };
        Duration::from_nanos(duration)
    }
}

impl From<FrameRate> for Frequency {
    fn from(frame_rate: FrameRate) -> Self {
        match frame_rate {
            FrameRate::SixtyHz => Self::SixtyHz,
            FrameRate::FiftyHz => Self::FiftyHz,
        }
    }
}
