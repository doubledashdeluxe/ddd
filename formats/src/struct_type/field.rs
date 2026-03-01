use crate::data_type::DataType;

pub struct Field<T: DataType> {
    name: &'static str,
    data_type: T,
}

impl<T: DataType> Field<T> {
    pub const fn new(name: &'static str, data_type: T) -> Self {
        Self { name, data_type }
    }

    pub const fn name(&self) -> &'static str {
        self.name
    }

    pub const fn data_type(&self) -> &T {
        &self.data_type
    }
}
