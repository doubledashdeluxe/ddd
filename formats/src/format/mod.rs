pub use crate::format::constant_list::ConstantList;
pub use crate::format::type_list::TypeList;

use crate::complex_data_type::ComplexDataType;
use crate::constant::Constant;

mod constant_list;
mod type_list;

pub struct Format {
    name: &'static str,
    constant_list: Box<dyn ConstantList>,
    type_list: Box<dyn TypeList>,
}

impl Format {
    pub fn new(name: &'static str) -> Self {
        Self { name, constant_list: Box::new(()), type_list: Box::new(()) }
    }
}

impl Format {
    pub fn with_constant<C: Constant + 'static>(self, constant: C) -> Self {
        let name = self.name;
        let constant_list = Box::new((self.constant_list, constant));
        let type_list = self.type_list;
        Self { name, constant_list, type_list }
    }

    pub fn with_type<T: ComplexDataType + 'static>(self, data_type: T) -> Self {
        let name = self.name;
        let constant_list = self.constant_list;
        let type_list = Box::new((self.type_list, data_type));
        Self { name, constant_list, type_list }
    }

    pub fn rs(&self) -> String {
        format!("{}{}", self.constant_list.rs(), self.type_list.rs()).trim().to_owned()
    }

    pub fn hh(&self) -> String {
        format!(
            concat!("#pragma once\n", "\n", "#include <portable/Types.hh>\n", "{}{}"),
            self.constant_list.hh(),
            self.type_list.hh(),
        )
    }

    pub fn cc(&self) -> String {
        format!(
            concat!("#include \"{}.hh\"\n", "\n", "#include <portable/Bytes.hh>\n", "{}"),
            self.name,
            self.type_list.cc(),
        )
    }
}
