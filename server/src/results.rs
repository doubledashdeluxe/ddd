use heapless::Vec;

use crate::formats::online::*;
use crate::kart::Kart;

pub fn compute(
    karts: &Vec<Kart, MAX_ROOM_KART_COUNT>,
    race_karts: &Vec<Option<ServerRaceKart>, MAX_ROOM_KART_COUNT>,
) -> Vec<ServerResult, MAX_ROOM_KART_COUNT> {
    let mut results: Vec<_, _> = karts
        .iter()
        .enumerate()
        .map(|(i, kart)| {
            let result_time = match &race_karts[i] {
                Some(race_kart) if race_kart.lap == 0 => race_kart.time,
                _ => MAX_TIME,
            };
            ServerResult { kart_index: i as u8, result_time, points: kart.points }
        })
        .collect();

    results.sort_unstable_by_key(|result| result.result_time);

    for (i, result) in results.iter_mut().enumerate() {
        result.points += POINTS[karts.len() - 2][i];
    }

    results
}

const POINTS: [&[u16]; 7] = [
    &[10, 0],
    &[10, 4, 0],
    &[10, 6, 3, 0],
    &[10, 7, 4, 2, 0],
    &[10, 7, 5, 3, 1, 0],
    &[10, 8, 6, 4, 2, 1, 0],
    &[10, 8, 6, 4, 3, 2, 1, 0],
];
