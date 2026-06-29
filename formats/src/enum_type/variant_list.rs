use crate::array_type::ArrayIndices;
use crate::data_type::DataType;
use crate::enum_type::variant::Variant;
use crate::str_ext::StrExt;

pub trait VariantList {
    fn count() -> u8;
    fn min_len(&self) -> usize;
    fn max_len(&self) -> usize;
    fn rs_variants(&self) -> String;
    fn rs_read(&self) -> String;
    fn rs_write(&self) -> String;
    fn hh_writer_decls(&self) -> String;
    fn hh_writer_defs(&self, enum_name: &str) -> String;
    fn hh_is_valid(&self) -> String;
    fn hh_read(&self) -> String;
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
        String::new()
    }

    fn rs_read(&self) -> String {
        String::new()
    }

    fn rs_write(&self) -> String {
        String::new()
    }

    fn hh_writer_decls(&self) -> String {
        String::new()
    }

    fn hh_writer_defs(&self, _: &str) -> String {
        String::new()
    }

    fn hh_is_valid(&self) -> String {
        String::new()
    }

    fn hh_read(&self) -> String {
        String::new()
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

    fn rs_read(&self) -> String {
        let variable_name = self.1.name().to_ascii_snake_case();
        format!(
            concat!(
                "{}",
                "            {} => {{\n",
                "                {}\n",
                "                Ok((Self::{}({}), buf))\n",
                "            }}\n",
            ),
            self.0.rs_read(),
            L::count(),
            self.1.data_type().rs_read(&variable_name).replace('\n', "\n                "),
            self.1.name(),
            &variable_name,
        )
    }

    fn rs_write(&self) -> String {
        let variable_name = self.1.name().to_ascii_snake_case();
        format!(
            concat!(
                "{}",
                "            Self::{}({}) => {{\n",
                "                *discriminant = {};\n",
                "                {}\n",
                "                Ok(buf)\n",
                "            }}\n",
            ),
            self.0.rs_write(),
            self.1.name(),
            &variable_name,
            L::count(),
            self.1.data_type().rs_write(&variable_name).replace('\n', "\n                "),
        )
    }

    fn hh_writer_decls(&self) -> String {
        format!(concat!("{}", "    class {};\n"), self.0.hh_writer_decls(), self.1.name())
    }

    fn hh_writer_defs(&self, enum_name: &str) -> String {
        format!(
            concat!(
                "{}\n",
                "template <typename D>\n",
                "class {}Writer<D>::{} : public {}Writer {{\n",
                "public:\n",
                "    bool write(u8 *buffer, u32 size, u32 &offset) override {{\n",
                "        MaybeUnused(buffer);\n",
                "        MaybeUnused(size);\n",
                "        MaybeUnused(offset);\n",
                "        D *d = static_cast<D *>(this);\n",
                "        if (offset + 1 > size) {{\n",
                "            return false;\n",
                "        }}\n",
                "        buffer[offset++] = {};\n",
                "{}\n",
                "        return true;\n",
                "    }}\n",
                "\n",
                "    friend D;\n",
                "\n",
                "private:\n",
                "    {}() {{}}\n",
                "}};\n",
            ),
            self.0.hh_writer_defs(enum_name),
            enum_name,
            self.1.name(),
            enum_name,
            L::count(),
            self.1.data_type().hh_write(self.1.name(), ArrayIndices::new(self.1.name())),
            self.1.name(),
        )
    }

    fn hh_is_valid(&self) -> String {
        format!(
            concat!(
                "{}",
                "        case {}:\n",
                "            {{\n",
                "        {}\n",
                "                return true;\n",
                "            }}\n",
            ),
            self.0.hh_is_valid(),
            L::count(),
            self.1
                .data_type()
                .hh_is_valid(self.1.name(), ArrayIndices::new(self.1.name()))
                .replace('\n', "\n        "),
        )
    }

    fn hh_read(&self) -> String {
        format!(
            concat!(
                "{}",
                "        case {}:\n",
                "            {{\n",
                "        {}\n",
                "                break;\n",
                "            }}\n"
            ),
            self.0.hh_read(),
            L::count(),
            self.1
                .data_type()
                .hh_read(self.1.name(), ArrayIndices::new(self.1.name()))
                .replace('\n', "\n        "),
        )
    }
}
