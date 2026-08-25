use anyhow::{Context, Result};
use heapless::Vec;
use orion::hazardous::hash::blake2::blake2b::Hasher::Blake2b256;

use crate::formats::online::MAX_COURSE_COUNT;

#[derive(Clone, Debug)]
pub struct Pack {
    courses: Vec<[u8; 32], MAX_COURSE_COUNT>,
    hash: [u8; 32],
}

impl Pack {
    pub fn new(courses: &[[u8; 32]], course_indices: &Vec<u8, MAX_COURSE_COUNT>) -> Result<Self> {
        let courses: Result<_> = course_indices
            .iter()
            .map(|i| courses.get(*i as usize).context("invalid course index").copied())
            .collect();
        let courses: Vec<_, _> = courses?;
        let hash = Blake2b256.digest(courses.as_flattened()).unwrap();
        Ok(Self { courses, hash: *hash.as_ref().as_array().unwrap() })
    }

    pub fn courses(&self) -> &[[u8; 32]] {
        &self.courses
    }

    pub const fn hash(&self) -> &[u8; 32] {
        &self.hash
    }
}
