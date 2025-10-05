use crate::constant::Constant;

pub trait ConstantList {
    fn rs(&self) -> String;
    fn hh(&self) -> String;
}

impl ConstantList for () {
    fn rs(&self) -> String {
        "".to_owned()
    }

    fn hh(&self) -> String {
        "\n".to_owned()
    }
}

impl<L: ConstantList, C: Constant> ConstantList for (L, C) {
    fn rs(&self) -> String {
        format!("{}{}", self.0.rs(), self.1.rs())
    }

    fn hh(&self) -> String {
        format!("{}{}", self.0.hh(), self.1.hh())
    }
}
