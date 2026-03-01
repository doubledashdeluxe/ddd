use crate::constant::Constant;
use crate::str_ext::StrExt;

pub struct StringConstant {
    name: &'static str,
    value: &'static str,
}

impl StringConstant {
    pub const fn new(name: &'static str, value: &'static str) -> Self {
        Self { name, value }
    }
}

impl Constant for StringConstant {
    fn rs(&self) -> String {
        format!("pub const {}: &str = \"{}\";\n", self.name, self.value)
    }

    fn hh(&self) -> String {
        format!(
            "const char {}[] = \"{}\";\n",
            self.name.to_ascii_lowercase().to_ascii_camel_case().to_ascii_sentence_case(),
            self.value
        )
    }
}
