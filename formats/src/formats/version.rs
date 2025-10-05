use std::array;

use crate::constant::Constant;
use crate::format::{ConstantList, Format, TypeList};
use crate::simple_constant::SimpleConstant;

pub fn format() -> Format<impl ConstantList, impl TypeList> {
    let version = include_str!("../../../version.txt");
    let version = version.strip_prefix("v").unwrap().trim_end();
    let mut parts = version.split('.');
    let [major, minor, patch] = array::from_fn(|_| parts.next().unwrap().parse().unwrap());
    assert!(parts.next().is_none());

    Format::new("Version")
        .with_constant(protocol_version())
        .with_constant(major_version(major))
        .with_constant(minor_version(minor))
        .with_constant(patch_version(patch))
}

fn protocol_version() -> impl Constant {
    SimpleConstant::new("PROTOCOL_VERSION", 0u32)
}

fn major_version(major: u8) -> impl Constant {
    SimpleConstant::new("MAJOR_VERSION", major)
}

fn minor_version(minor: u8) -> impl Constant {
    SimpleConstant::new("MINOR_VERSION", minor)
}

fn patch_version(patch: u8) -> impl Constant {
    SimpleConstant::new("PATCH_VERSION", patch)
}
