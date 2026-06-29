use crate::array_type::ArrayIndices;

pub trait DataType {
    fn min_len(&self) -> usize;
    fn max_len(&self) -> usize;
    fn rs_name(&self) -> String;
    fn rs_read(&self, name: &str) -> String;
    fn rs_write(&self, name: &str) -> String;
    fn hh_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String;
    fn hh_read(&self, name: &str, array_indices: ArrayIndices) -> String;
    fn hh_write(&self, name: &str, array_indices: ArrayIndices) -> String;
}
