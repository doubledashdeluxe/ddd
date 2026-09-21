use crate::storage::{Batch, Stats};

#[derive(Debug)]
pub enum Message {
    Batch(Batch),
    Stats(Stats),
}
