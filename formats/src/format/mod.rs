pub use crate::format::constant_list::ConstantList;
pub use crate::format::type_list::TypeList;

use crate::complex_data_type::ComplexDataType;
use crate::constant::Constant;

mod constant_list;
mod type_list;

pub struct Format<CL: ConstantList, TL: TypeList> {
    name: &'static str,
    constant_list: CL,
    type_list: TL,
}

impl Format<(), ()> {
    pub fn new(name: &'static str) -> Format<(), ()> {
        Format { name, constant_list: (), type_list: () }
    }
}

impl<CL: ConstantList, TL: TypeList> Format<CL, TL> {
    pub fn with_constant<C: Constant>(self, constant: C) -> Format<(CL, C), TL> {
        let name = self.name;
        let constant_list = (self.constant_list, constant);
        let type_list = self.type_list;
        Format { name, constant_list, type_list }
    }

    pub fn with_type<T: ComplexDataType>(self, data_type: T) -> Format<CL, (TL, T)> {
        let name = self.name;
        let constant_list = self.constant_list;
        let type_list = (self.type_list, data_type);
        Format { name, constant_list, type_list }
    }

    pub fn rs(&self) -> String {
        format!("{}{}", self.constant_list.rs(), self.type_list.rs()).trim().to_owned()
    }

    pub fn hh(&self) -> String {
        format!(
            concat!("#pragma once\n", "\n", "#include <portable/Types.hh>\n", "{}{}"),
            self.constant_list.hh().trim_end(),
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
