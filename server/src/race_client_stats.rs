#[derive(Clone, Copy, Debug, Default)]
pub struct RaceClientStats {
    delayed_frames: Diff,
    latency: Mean,
    stability: Mean,
}

impl RaceClientStats {
    pub fn update(&mut self, delayed_frames: u32, latency: u16, stability: u8) {
        self.delayed_frames.update(delayed_frames);
        self.latency.update(latency);
        self.stability.update(stability);
    }

    pub const fn delayed_frames(&self) -> Option<u64> {
        self.delayed_frames.diff()
    }

    pub const fn latency(&self) -> Option<u64> {
        self.latency.mean()
    }

    pub const fn stability(&self) -> Option<u64> {
        self.stability.mean()
    }
}

#[derive(Clone, Copy, Debug, Default)]
struct Diff {
    min: Option<u64>,
    max: Option<u64>,
}

impl Diff {
    fn update(&mut self, value: impl Into<u64>) {
        let value = value.into();
        self.min = Some(self.min.map_or(value, |min| min.min(value)));
        self.max = Some(self.max.map_or(value, |max| max.max(value)));
    }

    const fn diff(&self) -> Option<u64> {
        match (self.min, self.max) {
            (Some(min), Some(max)) => Some(max - min),
            _ => None,
        }
    }
}

#[derive(Clone, Copy, Debug, Default)]
struct Mean {
    sum: u64,
    count: u64,
}

impl Mean {
    fn update(&mut self, value: impl Into<u64>) {
        self.sum += value.into();
        self.count += 1;
    }

    const fn mean(&self) -> Option<u64> {
        self.sum.checked_div(self.count)
    }
}
