use crate::format::Format;
use crate::string_constant::StringConstant;

pub fn format() -> Format {
    let version = include_str!("../../../version.txt").trim_end();
    let version = StringConstant::new("VERSION", version);
    Format::new("Version").with_constant(version)
}
