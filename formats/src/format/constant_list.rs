use crate::constant::Constant;

pub trait ConstantList {
    fn rs(&self) -> String;
    fn hh(&self) -> String;
}

impl ConstantList for () {
    fn rs(&self) -> String {
        String::new()
    }

    fn hh(&self) -> String {
        String::new()
    }
}

impl<L: ConstantList, C: Constant> ConstantList for (L, C) {
    fn rs(&self) -> String {
        format!("{}{}", self.0.rs(), self.1.rs())
    }

    fn hh(&self) -> String {
        format!("{}\n{}", self.0.hh().trim_end(), self.1.hh())
    }
}
