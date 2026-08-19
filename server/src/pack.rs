use serde::{Deserialize, Serialize};

use crate::storage::base64;

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq, Serialize, Deserialize)]
pub struct Pack {
    pub course_count: u8,
    #[serde(with = "base64")]
    pub hash: [u8; 32],
}
