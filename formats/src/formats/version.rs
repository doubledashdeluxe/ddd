use crate::format::{ConstantList, Format, TypeList};
use crate::simple_constant::SimpleConstant;
use crate::string_constant::StringConstant;

pub fn format() -> Format<impl ConstantList, impl TypeList> {
    let protocol_version = SimpleConstant::new("PROTOCOL_VERSION", 3u32);
    let version = include_str!("../../../version.txt").trim_end();
    let version = StringConstant::new("VERSION", version);
    Format::new("Version").with_constant(protocol_version).with_constant(version)
}
