use crate::data_type::DataType;

pub struct Field<T: DataType> {
    name: &'static str,
    data_type: T,
}

impl<T: DataType> Field<T> {
    pub fn new(name: &'static str, data_type: T) -> Field<T> {
        Field { name, data_type }
    }

    pub fn name(&self) -> &'static str {
        self.name
    }

    pub fn data_type(&self) -> &T {
        &self.data_type
    }
}
