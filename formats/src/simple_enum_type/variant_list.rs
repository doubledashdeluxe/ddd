use crate::simple_enum_type::variant::Variant;

pub trait VariantList {
    fn count() -> u8;
    fn rs_variants(&self) -> String;
    fn rs_read(&self, enum_name: &str) -> String;
    fn rs_write(&self, enum_name: &str) -> String;
    fn hh_variants(&self) -> String;
    fn cc_cases(&self) -> String;
}

impl VariantList for () {
    fn count() -> u8 {
        0
    }

    fn rs_variants(&self) -> String {
        "".to_owned()
    }

    fn rs_read(&self, _: &str) -> String {
        "".to_owned()
    }

    fn rs_write(&self, _: &str) -> String {
        "".to_owned()
    }

    fn hh_variants(&self) -> String {
        "".to_owned()
    }

    fn cc_cases(&self) -> String {
        "".to_owned()
    }
}

impl<L: VariantList> VariantList for (L, Variant) {
    fn count() -> u8 {
        L::count().checked_add(1).unwrap()
    }

    fn rs_variants(&self) -> String {
        format!("{}    {},\n", self.0.rs_variants(), self.1.name())
    }

    fn rs_read(&self, enum_name: &str) -> String {
        format!(
            concat!("{}", "            {} => Ok(({}::{}, buf)),\n",),
            self.0.rs_read(enum_name),
            L::count(),
            enum_name,
            self.1.name(),
        )
    }

    fn rs_write(&self, enum_name: &str) -> String {
        format!(
            concat!("{}", "            {}::{} => {},\n",),
            self.0.rs_write(enum_name),
            enum_name,
            self.1.name(),
            L::count(),
        )
    }

    fn hh_variants(&self) -> String {
        format!("{}        {} = {},\n", self.0.hh_variants(), self.1.name(), L::count())
    }

    fn cc_cases(&self) -> String {
        format!(concat!("{}", "    case {}:\n",), self.0.cc_cases(), L::count())
    }
}
