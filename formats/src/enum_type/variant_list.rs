use crate::array_type::ArrayIndices;
use crate::data_type::DataType;
use crate::enum_type::variant::Variant;
use crate::str_ext::StrExt;

pub trait VariantList {
    fn count() -> u8;
    fn min_len(&self) -> usize;
    fn max_len(&self) -> usize;
    fn rs_variants(&self) -> String;
    fn rs_read(&self, enum_name: &str) -> String;
    fn rs_write(&self, enum_name: &str) -> String;
    fn hh_read_delegates(&self) -> String;
    fn hh_writer_decls(&self) -> String;
    fn hh_writer_defs(&self, enum_name: &str) -> String;
    fn cc_is_valid(&self) -> String;
    fn cc_read(&self) -> String;
    fn cc_writers(&self, enum_name: &str) -> String;
}

impl VariantList for () {
    fn count() -> u8 {
        0
    }

    fn min_len(&self) -> usize {
        usize::MAX
    }

    fn max_len(&self) -> usize {
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

    fn hh_read_delegates(&self) -> String {
        "".to_owned()
    }

    fn hh_writer_decls(&self) -> String {
        "".to_owned()
    }

    fn hh_writer_defs(&self, _: &str) -> String {
        "".to_owned()
    }

    fn cc_is_valid(&self) -> String {
        "".to_owned()
    }

    fn cc_read(&self) -> String {
        "".to_owned()
    }

    fn cc_writers(&self, _: &str) -> String {
        "".to_owned()
    }
}

impl<L: VariantList, T: DataType> VariantList for (L, Variant<T>) {
    fn count() -> u8 {
        L::count().checked_add(1).unwrap()
    }

    fn min_len(&self) -> usize {
        self.0.min_len().min(self.1.data_type().min_len())
    }

    fn max_len(&self) -> usize {
        self.0.max_len().max(self.1.data_type().max_len())
    }

    fn rs_variants(&self) -> String {
        format!(
            "{}    {}({}),\n",
            self.0.rs_variants(),
            self.1.name(),
            self.1.data_type().rs_name(),
        )
    }

    fn rs_read(&self, enum_name: &str) -> String {
        let variable_name = self.1.name().to_ascii_snake_case();
        format!(
            concat!(
                "{}",
                "            {} => {{\n",
                "                {}\n",
                "                Ok(({}::{}({}), buf))\n",
                "            }}\n",
            ),
            self.0.rs_read(enum_name),
            L::count(),
            self.1.data_type().rs_read(&variable_name).replace("\n", "\n                "),
            enum_name,
            self.1.name(),
            &variable_name,
        )
    }

    fn rs_write(&self, enum_name: &str) -> String {
        let variable_name = self.1.name().to_ascii_snake_case();
        format!(
            concat!(
                "{}",
                "            {}::{}({}) => {{\n",
                "                *discriminant = {};\n",
                "                {}\n",
                "                Ok(buf)\n",
                "            }}\n",
            ),
            self.0.rs_write(enum_name),
            enum_name,
            self.1.name(),
            &variable_name,
            L::count(),
            self.1.data_type().rs_write(&variable_name).replace("\n", "\n                "),
        )
    }

    fn hh_read_delegates(&self) -> String {
        format!(
            concat!("{}", "{}"),
            self.0.hh_read_delegates(),
            self.1.data_type().hh_read_delegate(self.1.name(), ArrayIndices::new()),
        )
    }

    fn hh_writer_decls(&self) -> String {
        format!(concat!("{}", "    class {};\n"), self.0.hh_writer_decls(), self.1.name())
    }

    fn hh_writer_defs(&self, enum_name: &str) -> String {
        format!(
            concat!(
                "{}\n",
                "class {}Writer::{} : public {}Writer {{\n",
                "public:\n",
                "    bool write(u8 *buffer, u32 size, u32 &offset) override;\n",
                "\n",
                "private:\n",
                "{}",
                "}};\n",
            ),
            self.0.hh_writer_defs(enum_name),
            enum_name,
            self.1.name(),
            enum_name,
            self.1.data_type().hh_write_delegate(self.1.name(), ArrayIndices::new()),
        )
    }

    fn cc_is_valid(&self) -> String {
        format!(
            concat!(
                "{}",
                "    case {}:\n",
                "        {{\n",
                "        {}\n",
                "            return true;\n",
                "        }}\n",
            ),
            self.0.cc_is_valid(),
            L::count(),
            self.1
                .data_type()
                .cc_is_valid(self.1.name(), ArrayIndices::new())
                .replace("\n", "\n        "),
        )
    }

    fn cc_read(&self) -> String {
        format!(
            concat!(
                "{}",
                "    case {}:\n",
                "        {{\n",
                "        {}\n",
                "            break;\n",
                "        }}\n"
            ),
            self.0.cc_read(),
            L::count(),
            self.1
                .data_type()
                .cc_read(self.1.name(), ArrayIndices::new())
                .replace("\n", "\n        "),
        )
    }

    fn cc_writers(&self, enum_name: &str) -> String {
        format!(
            concat!(
                "{}\n",
                "bool {}Writer::{}::write(u8 *buffer, u32 size, u32 &offset) {{\n",
                "    if (offset + 1 > size) {{\n",
                "        return false;\n",
                "    }}\n",
                "    buffer[offset++] = {};\n",
                "{}\n",
                "    return true;\n",
                "}}\n",
            ),
            self.0.cc_writers(enum_name),
            enum_name,
            self.1.name(),
            L::count(),
            self.1.data_type().cc_write(self.1.name(), ArrayIndices::new()),
        )
    }
}
