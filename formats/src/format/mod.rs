pub use crate::format::type_list::TypeList;

use crate::complex_data_type::ComplexDataType;

mod type_list;

pub struct Format<L: TypeList> {
    name: &'static str,
    list: L,
}

impl Format<()> {
    pub fn new(name: &'static str) -> Format<()> {
        Format { name, list: () }
    }
}

impl<L: TypeList> Format<L> {
    pub fn with_type<T: ComplexDataType>(self, data_type: T) -> Format<(L, T)> {
        Format { name: self.name, list: (self.list, data_type) }
    }

    pub fn rs(&self) -> String {
        self.list.rs().trim().to_owned()
    }

    pub fn hh(&self) -> String {
        format!(
            concat!("#pragma once\n", "\n", "#include <common/Types.hh>\n", "{}"),
            self.list.hh(),
        )
    }

    pub fn cc(&self) -> String {
        format!(
            concat!("#include \"{}.hh\"\n", "\n", "#include <common/Bytes.hh>\n", "{}"),
            self.name,
            self.list.cc(),
        )
    }
}
