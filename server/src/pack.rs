use heapless::Vec;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Pack {
    pub course_count: u8,
    pub hash: Vec<u8, 32>,
}
