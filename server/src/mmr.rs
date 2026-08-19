pub fn mmr(mmrs: impl IntoIterator<Item = u16>) -> u16 {
    let mut count = 0;
    let sum: u32 = mmrs.into_iter().map(u32::from).inspect(|_| count += 1).sum();
    sum.checked_div(count).unwrap_or(0) as u16
}
