use crate::data_type::DataType;

pub struct Variant<T: DataType> {
    name: &'static str,
    data_type: T,
}

impl<T: DataType> Variant<T> {
    pub fn new(name: &'static str, data_type: T) -> Variant<T> {
        Variant { name, data_type }
    }

    pub fn name(&self) -> &'static str {
        self.name
    }

    pub fn data_type(&self) -> &T {
        &self.data_type
    }
}
