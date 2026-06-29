use crate::complex_data_type::ComplexDataType;

pub trait TypeList {
    fn rs(&self) -> String;
    fn hh(&self) -> String;
}

impl TypeList for () {
    fn rs(&self) -> String {
        String::new()
    }

    fn hh(&self) -> String {
        String::new()
    }
}

impl<T: ComplexDataType> TypeList for (Box<dyn TypeList>, T) {
    fn rs(&self) -> String {
        format!("{}\n{}", self.0.rs(), self.1.rs())
    }

    fn hh(&self) -> String {
        format!("{}\n{}", self.0.hh(), self.1.hh())
    }
}
