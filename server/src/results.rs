use heapless::Vec;

use crate::formats::online::*;
use crate::kart::Kart;

pub fn compute(
    karts: &mut Vec<Kart, MAX_ROOM_KART_COUNT>,
    race_karts: &Vec<Option<ServerRaceKart>, MAX_ROOM_KART_COUNT>,
) -> Vec<ServerResult, MAX_ROOM_KART_COUNT> {
    let mut results: Vec<_, _> = karts
        .iter()
        .enumerate()
        .map(|(i, kart)| ServerResult { kart_index: i as u8, points: kart.points })
        .collect();

    results.sort_unstable_by_key(|result| {
        let kart = &race_karts[result.kart_index as usize];
        let kart = kart.as_ref().filter(|kart| kart.lap == 0);
        kart.map_or(MAX_TIME, |kart| kart.time)
    });

    for (i, result) in results.iter_mut().enumerate() {
        result.points += POINTS[karts.len() - 2][i];
        karts[result.kart_index as usize].points = result.points;
    }

    results
}

const POINTS: [[u16; 8]; 7] = points();

const fn points() -> [[u16; 8]; 7] {
    let mut points = [[0; _]; _];

    points[6] = [10, 8, 6, 4, 3, 2, 1, 0];

    let mut i = 0u32;
    while i < 6 {
        let mut j = 0u32;
        while j < i + 1 {
            let t = (j * 7) as f64 / (i + 1) as f64;

            let t0 = (t.floor() as isize).cast_unsigned();
            let t1 = t0 + 1;

            let p0 = points[6][t0] as f64;
            let p1 = points[6][t1] as f64;

            let p = p0 * (1.0 - t.fract()) + p1 * t.fract();
            points[i as usize][j as usize] = (p as i16).cast_unsigned();

            j += 1;
        }
        i += 1;
    }

    points
}
