#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Pack {
    pub course_count: u8,
    pub hash: [u8; 32],
}
