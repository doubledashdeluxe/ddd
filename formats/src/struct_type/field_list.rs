use crate::array_type::ArrayIndices;
use crate::data_type::DataType;
use crate::str_ext::StrExt;
use crate::struct_type::field::Field;

pub trait FieldList {
    fn min_len(&self) -> usize;
    fn max_len(&self) -> usize;
    fn rs_fields(&self) -> String;
    fn rs_field_names(&self) -> String;
    fn rs_read(&self) -> String;
    fn rs_write(&self) -> String;
    fn hh_read_delegates(&self) -> String;
    fn hh_write_delegates(&self) -> String;
    fn cc_is_valid(&self) -> String;
    fn cc_read(&self) -> String;
    fn cc_write(&self) -> String;
}

impl FieldList for () {
    fn min_len(&self) -> usize {
        0
    }

    fn max_len(&self) -> usize {
        0
    }

    fn rs_fields(&self) -> String {
        "".to_owned()
    }

    fn rs_field_names(&self) -> String {
        "".to_owned()
    }

    fn rs_read(&self) -> String {
        "".to_owned()
    }

    fn rs_write(&self) -> String {
        "".to_owned()
    }

    fn hh_read_delegates(&self) -> String {
        "".to_owned()
    }

    fn hh_write_delegates(&self) -> String {
        "".to_owned()
    }

    fn cc_is_valid(&self) -> String {
        "".to_owned()
    }

    fn cc_read(&self) -> String {
        "".to_owned()
    }

    fn cc_write(&self) -> String {
        "".to_owned()
    }
}

impl<L: FieldList, T: DataType> FieldList for (L, Field<T>) {
    fn min_len(&self) -> usize {
        self.0.min_len() + self.1.data_type().min_len()
    }

    fn max_len(&self) -> usize {
        self.0.max_len() + self.1.data_type().max_len()
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
            self.1.data_type().rs_read(self.1.name()).replace("\n", "\n        "),
        )
    }

    fn rs_write(&self) -> String {
        format!(
            concat!("{}", "        {}\n"),
            self.0.rs_write(),
            self.1.data_type().rs_write(self.1.name()).replace("\n", "\n        "),
        )
    }

    fn hh_read_delegates(&self) -> String {
        let name = self.1.name().to_ascii_sentence_case();
        format!(
            concat!("{}", "{}"),
            self.0.hh_read_delegates(),
            self.1.data_type().hh_read_delegate(&name, ArrayIndices::new()),
        )
    }

    fn hh_write_delegates(&self) -> String {
        let name = self.1.name().to_ascii_sentence_case();
        format!(
            concat!("{}", "{}"),
            self.0.hh_write_delegates(),
            self.1.data_type().hh_write_delegate(&name, ArrayIndices::new()),
        )
    }

    fn cc_is_valid(&self) -> String {
        let name = self.1.name().to_ascii_sentence_case();
        format!(
            concat!("{}", "{}\n"),
            self.0.cc_is_valid(),
            self.1.data_type().cc_is_valid(&name, ArrayIndices::new()),
        )
    }

    fn cc_read(&self) -> String {
        let name = self.1.name().to_ascii_sentence_case();
        format!(
            concat!("{}", "{}\n"),
            self.0.cc_read(),
            self.1.data_type().cc_read(&name, ArrayIndices::new()),
        )
    }

    fn cc_write(&self) -> String {
        let name = self.1.name().to_ascii_sentence_case();
        format!(
            concat!("{}", "{}\n"),
            self.0.cc_write(),
            self.1.data_type().cc_write(&name, ArrayIndices::new()),
        )
    }
}
