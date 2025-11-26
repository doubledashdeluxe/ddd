pub trait Mmr {
    fn mmr(&self) -> u16;
}

impl<T: Mmr> Mmr for [T] {
    fn mmr(&self) -> u16 {
        let mmr: u32 = self.iter().map(|kart| kart.mmr() as u32).sum();
        let mmr = mmr / self.len() as u32;
        mmr as u16
    }
}
