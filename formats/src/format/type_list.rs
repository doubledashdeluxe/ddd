use crate::complex_data_type::ComplexDataType;

pub trait TypeList {
    fn rs(&self) -> String;
    fn hh(&self) -> String;
    fn cc(&self) -> String;
}

impl TypeList for () {
    fn rs(&self) -> String {
        "".to_owned()
    }

    fn hh(&self) -> String {
        "".to_owned()
    }

    fn cc(&self) -> String {
        "".to_owned()
    }
}

impl<L: TypeList, T: ComplexDataType> TypeList for (L, T) {
    fn rs(&self) -> String {
        format!("{}\n{}", self.0.rs(), self.1.rs())
    }

    fn hh(&self) -> String {
        format!("{}\n{}", self.0.hh(), self.1.hh())
    }

    fn cc(&self) -> String {
        format!("{}\n{}", self.0.cc(), self.1.cc())
    }
}
