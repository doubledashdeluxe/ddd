use std::array;

use rand::Rng;

use crate::formats::online::{CharacterId, ItemId, ModeIndex, RoomOptionItemMode};
use crate::item_weights::*;

pub fn choose(
    kart_count: usize,
    mode_index: ModeIndex,
    item_mode: ItemMode,
    character_id: CharacterId,
    other_character_id: CharacterId,
    rank: u8,
    other_item: ItemId,
    item_counts: [u8; 16],
    rng: &mut impl Rng,
) -> ItemId {
    let weights = match (mode_index, item_mode, kart_count) {
        (ModeIndex::Versus, ItemMode::Recommended, 2) => &VERSUS_RECOMMENDED_2P,
        (ModeIndex::Versus, ItemMode::Recommended, 3) => &VERSUS_RECOMMENDED_3P,
        (ModeIndex::Versus, ItemMode::Recommended, 4) => &VERSUS_RECOMMENDED_4P,
        (ModeIndex::Versus, ItemMode::Recommended, _) => &VERSUS_RECOMMENDED_8P,
        (ModeIndex::Versus, ItemMode::Basic, 2) => &VERSUS_BASIC_2P,
        (ModeIndex::Versus, ItemMode::Basic, 3) => &VERSUS_BASIC_3P,
        (ModeIndex::Versus, ItemMode::Basic, 4) => &VERSUS_BASIC_4P,
        (ModeIndex::Versus, ItemMode::Basic, _) => &VERSUS_BASIC_8P,
        (ModeIndex::Versus, ItemMode::Frantic, 2) => &VERSUS_FRANTIC_2P,
        (ModeIndex::Versus, ItemMode::Frantic, 3) => &VERSUS_FRANTIC_3P,
        (ModeIndex::Versus, ItemMode::Frantic, 4) => &VERSUS_FRANTIC_4P,
        (ModeIndex::Versus, ItemMode::Frantic, _) => &VERSUS_FRANTIC_8P,
        (ModeIndex::Versus, ItemMode::None, _) => return ItemId::None,
        (ModeIndex::Balloon, _, _) => &BALLOON,
        (ModeIndex::Escape, _, _) => &ESCAPE,
        (ModeIndex::Bomb, _, _) => return ItemId::Bomb,
        (ModeIndex::TimeAttack, _, _) => return ItemId::None,
    };

    let rank = match (mode_index, kart_count) {
        (ModeIndex::Versus, 5) if rank >= 1 => rank + 1,
        (ModeIndex::Versus, 5) if rank >= 3 => rank + 2,
        (ModeIndex::Versus, 6) if rank >= 2 => rank + 1,
        (ModeIndex::Versus, 6) if rank >= 4 => rank + 2,
        (ModeIndex::Versus, 7) if rank >= 4 => rank + 1,
        (ModeIndex::Balloon, _) => 0,
        (ModeIndex::Escape, _) => 0,
        _ => rank,
    };

    let special_item = match mode_index {
        ModeIndex::Balloon => ItemId::None,
        ModeIndex::Escape => ItemId::None,
        _ => character_id.special_item(),
    };
    let weight = |i| {
        let item: ItemId = ITEMS[i];
        let base = item.base();
        let count = item.count();
        if item_counts[base as usize] + count > base.max_count() {
            return 0;
        }

        let weight = |i| u32::from(weights[rank as usize][i]);

        let is_special = i >= 9;
        if is_special {
            if other_item.is_special() {
                return 0;
            }
            let weight = if special_item == ItemId::None {
                weight(i + 9)
            } else if item == special_item {
                weight(i)
            } else {
                return 0;
            };
            let other_special_item = match mode_index {
                ModeIndex::Balloon => return weight,
                ModeIndex::Escape => return weight,
                _ => other_character_id.special_item(),
            };
            if special_item == other_special_item { weight * 3 / 2 } else { weight }
        } else {
            if item == other_item && !item.can_have_two() {
                return 0;
            }
            weight(i)
        }
    };

    let mut sum = 0;
    let partial_sums: [_; 18] = array::from_fn(|i| {
        sum += weight(i);
        sum
    });

    let limit = rng.random_range(..sum);
    let index = partial_sums.partition_point(|partial_sum| *partial_sum <= limit);
    let item = ITEMS[index];
    if item == ItemId::TripleGreenShells
        && mode_index == ModeIndex::Versus
        && rng.random_ratio(3, 5)
    {
        return ItemId::TripleRedShells;
    }
    item
}

type ItemMode = RoomOptionItemMode;

const ITEMS: [ItemId; 18] = [
    ItemId::GreenShell,
    ItemId::RedShell,
    ItemId::BlueShell,
    ItemId::Banana,
    ItemId::Mushroom,
    ItemId::TripleMushrooms,
    ItemId::Star,
    ItemId::Lightning,
    ItemId::FakeItemBox,
    ItemId::TripleGreenShells,
    ItemId::BowserShell,
    ItemId::GiantBanana,
    ItemId::Fireballs,
    ItemId::Bomb,
    ItemId::YoshiEgg,
    ItemId::Chomp,
    ItemId::GoldenMushroom,
    ItemId::Heart,
];

#[cfg(test)]
mod tests {
    use std::array;

    use rand::SeedableRng;

    use crate::crypto::ChaCha20Rng;
    use crate::formats::online::{CharacterId, ItemId, ModeIndex};
    use crate::item::{self, ItemMode};

    #[test]
    fn choose() {
        fn choose(
            kart_count: usize,
            mode_index: ModeIndex,
            item_mode: ItemMode,
            character_id: CharacterId,
            other_character_id: CharacterId,
            rank: u8,
            other_item: ItemId,
            item_counts: [u8; 16],
            items: &[ItemId],
        ) {
            let mut rng = ChaCha20Rng::seed_from_u64(0);
            let mut counts = [0; 22];
            for _ in 0..1000 {
                let item = item::choose(
                    kart_count,
                    mode_index,
                    item_mode,
                    character_id,
                    other_character_id,
                    rank,
                    other_item,
                    item_counts,
                    &mut rng,
                );
                counts[item as usize] += 1;
            }
            for (i, count) in counts.iter().enumerate() {
                assert_eq!(
                    *count != 0,
                    items.iter().any(|item| *item as usize == i),
                    "{i} {count}"
                );
            }
        }

        choose(
            2,
            ModeIndex::Versus,
            ItemMode::Basic,
            CharacterId::Mario,
            CharacterId::Luigi,
            0,
            ItemId::None,
            [0; 16],
            &[ItemId::GreenShell, ItemId::Banana],
        );

        choose(
            2,
            ModeIndex::Versus,
            ItemMode::Basic,
            CharacterId::Mario,
            CharacterId::Luigi,
            0,
            ItemId::None,
            array::from_fn(|i| if i == ItemId::Banana as usize { 15 } else { 0 }),
            &[ItemId::GreenShell],
        );

        choose(
            2,
            ModeIndex::Versus,
            ItemMode::Basic,
            CharacterId::Mario,
            CharacterId::Luigi,
            1,
            ItemId::None,
            [0; 16],
            &[ItemId::GreenShell, ItemId::RedShell, ItemId::Mushroom, ItemId::TripleMushrooms],
        );

        choose(
            2,
            ModeIndex::Versus,
            ItemMode::Basic,
            CharacterId::Mario,
            CharacterId::Luigi,
            1,
            ItemId::TripleMushrooms,
            [0; 16],
            &[ItemId::GreenShell, ItemId::RedShell, ItemId::Mushroom],
        );

        choose(
            2,
            ModeIndex::Versus,
            ItemMode::Frantic,
            CharacterId::Mario,
            CharacterId::Luigi,
            0,
            ItemId::None,
            [0; 16],
            &[ItemId::RedShell, ItemId::FakeItemBox, ItemId::Fireballs],
        );

        choose(
            2,
            ModeIndex::Versus,
            ItemMode::Frantic,
            CharacterId::Mario,
            CharacterId::Luigi,
            0,
            ItemId::Fireballs,
            [0; 16],
            &[ItemId::RedShell, ItemId::FakeItemBox],
        );

        choose(
            2,
            ModeIndex::Versus,
            ItemMode::Frantic,
            CharacterId::Mario,
            CharacterId::Luigi,
            0,
            ItemId::TripleGreenShells,
            [0; 16],
            &[ItemId::RedShell, ItemId::FakeItemBox],
        );

        choose(
            2,
            ModeIndex::Versus,
            ItemMode::Frantic,
            CharacterId::Teresa,
            CharacterId::Pakkun,
            0,
            ItemId::None,
            [0; 16],
            &[
                ItemId::RedShell,
                ItemId::FakeItemBox,
                ItemId::TripleGreenShells,
                ItemId::TripleRedShells,
                ItemId::BowserShell,
                ItemId::GiantBanana,
                ItemId::Fireballs,
                ItemId::Bomb,
                ItemId::YoshiEgg,
            ],
        );

        choose(
            2,
            ModeIndex::Bomb,
            ItemMode::Recommended,
            CharacterId::Mario,
            CharacterId::Luigi,
            0,
            ItemId::None,
            [0; 16],
            &[ItemId::Bomb],
        );
    }
}
