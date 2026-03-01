use crate::simple_enum_type::variant::Variant;

pub trait VariantList {
    fn count() -> u8;
    fn rs_variants(&self) -> String;
    fn rs_read(&self) -> String;
    fn rs_write(&self) -> String;
    fn hh_variants(&self) -> String;
    fn cc_cases(&self) -> String;
}

impl VariantList for () {
    fn count() -> u8 {
        0
    }

    fn rs_variants(&self) -> String {
        String::new()
    }

    fn rs_read(&self) -> String {
        String::new()
    }

    fn rs_write(&self) -> String {
        String::new()
    }

    fn hh_variants(&self) -> String {
        String::new()
    }

    fn cc_cases(&self) -> String {
        String::new()
    }
}

impl<L: VariantList> VariantList for (L, Variant) {
    fn count() -> u8 {
        L::count().checked_add(1).unwrap()
    }

    fn rs_variants(&self) -> String {
        format!("{}    {},\n", self.0.rs_variants(), self.1.name())
    }

    fn rs_read(&self) -> String {
        format!(
            concat!("{}", "            {} => Ok((Self::{}, buf)),\n"),
            self.0.rs_read(),
            L::count(),
            self.1.name(),
        )
    }

    fn rs_write(&self) -> String {
        format!(
            concat!("{}", "            Self::{} => {},\n"),
            self.0.rs_write(),
            self.1.name(),
            L::count(),
        )
    }

    fn hh_variants(&self) -> String {
        format!("{}        {} = {},\n", self.0.hh_variants(), self.1.name(), L::count())
    }

    fn cc_cases(&self) -> String {
        format!(concat!("{}", "    case {}:\n"), self.0.cc_cases(), L::count())
    }
}
