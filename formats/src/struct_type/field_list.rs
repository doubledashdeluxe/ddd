use crate::array_type::ArrayIndices;
use crate::data_type::DataType;
use crate::str_ext::StrExt;
use crate::struct_type::field::Field;

pub trait FieldList {
    fn min_len(&self) -> usize;
    fn max_len(&self) -> usize;
    fn rs_const() -> &'static str;
    fn rs_fields(&self) -> String;
    fn rs_field_names(&self) -> String;
    fn rs_read(&self) -> String;
    fn rs_write(&self) -> String;
    fn hh_is_valid(&self) -> String;
    fn hh_read(&self) -> String;
    fn hh_write(&self) -> String;
}

impl FieldList for () {
    fn min_len(&self) -> usize {
        0
    }

    fn max_len(&self) -> usize {
        0
    }

    fn rs_const() -> &'static str {
        " const"
    }

    fn rs_fields(&self) -> String {
        String::new()
    }

    fn rs_field_names(&self) -> String {
        String::new()
    }

    fn rs_read(&self) -> String {
        String::new()
    }

    fn rs_write(&self) -> String {
        String::new()
    }

    fn hh_is_valid(&self) -> String {
        String::new()
    }

    fn hh_read(&self) -> String {
        String::new()
    }

    fn hh_write(&self) -> String {
        String::new()
    }
}

impl<L: FieldList, T: DataType> FieldList for (L, Field<T>) {
    fn min_len(&self) -> usize {
        self.0.min_len() + self.1.data_type().min_len()
    }

    fn max_len(&self) -> usize {
        self.0.max_len() + self.1.data_type().max_len()
    }

    fn rs_const() -> &'static str {
        ""
    }

    fn rs_fields(&self) -> String {
        format!(
            concat!("{}", "    pub {}: {},\n"),
            self.0.rs_fields(),
            self.1.name(),
            self.1.data_type().rs_name(),
        )
    }

    fn rs_field_names(&self) -> String {
        format!(concat!("{}", "            {},\n"), self.0.rs_field_names(), self.1.name())
    }

    fn rs_read(&self) -> String {
        format!(
            concat!("{}", "        {}\n"),
            self.0.rs_read(),
            self.1.data_type().rs_read(self.1.name()).replace('\n', "\n        "),
        )
    }

    fn rs_write(&self) -> String {
        format!(
            concat!("{}", "        {}\n"),
            self.0.rs_write(),
            self.1.data_type().rs_write(self.1.name()).replace('\n', "\n        "),
        )
    }

    fn hh_is_valid(&self) -> String {
        let name = self.1.name().to_ascii_sentence_case();
        format!(
            concat!("{}", "{}\n"),
            self.0.hh_is_valid(),
            self.1.data_type().hh_is_valid(&name, ArrayIndices::new(&name)),
        )
    }

    fn hh_read(&self) -> String {
        let name = self.1.name().to_ascii_sentence_case();
        format!(
            concat!("{}", "{}\n"),
            self.0.hh_read(),
            self.1.data_type().hh_read(&name, ArrayIndices::new(&name)),
        )
    }

    fn hh_write(&self) -> String {
        let name = self.1.name().to_ascii_sentence_case();
        format!(
            concat!("{}", "{}\n"),
            self.0.hh_write(),
            self.1.data_type().hh_write(&name, ArrayIndices::new(&name)),
        )
    }
}
