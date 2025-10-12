use crate::complex_data_type::ComplexDataType;
use crate::data_type::DataType;
use crate::enum_type::variant::Variant;
use crate::enum_type::variant_list::VariantList;

mod variant;
mod variant_list;

pub struct EnumType<L: VariantList> {
    name: &'static str,
    list: L,
}

impl EnumType<()> {
    pub fn new(name: &'static str) -> EnumType<()> {
        EnumType { name, list: () }
    }
}

impl<L: VariantList> EnumType<L> {
    pub fn with_variant<T: DataType>(
        self,
        name: &'static str,
        data_type: T,
    ) -> EnumType<(L, Variant<T>)> {
        EnumType { name: self.name, list: (self.list, Variant::new(name, data_type)) }
    }
}

impl<L: VariantList> ComplexDataType for EnumType<L> {
    fn min_len(&self) -> usize {
        1 + self.list.min_len()
    }

    fn max_len(&self) -> usize {
        1 + self.list.max_len()
    }

    fn name(&self) -> &'static str {
        self.name
    }

    fn rs(&self) -> String {
        format!(
            concat!(
                "#[derive(Clone, Debug)]\n",
                "pub enum {} {{\n",
                "{}}}\n",
                "\n",
                "impl {} {{\n",
                "    const MIN_LEN: usize = {};\n",
                "    const MAX_LEN: usize = {};\n",
                "\n",
                "    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {{\n",
                "        let (discriminant, buf) = buf.split_first().ok_or(())?;\n",
                "        match discriminant {{\n",
                "{}",
                "            _ => Err(()),\n",
                "        }}\n",
                "    }}\n",
                "\n",
                "    pub fn write<'a>(&self, buf: &'a mut [u8]) -> Result<&'a mut [u8], ()> {{\n",
                "        let (discriminant, buf) = buf.split_first_mut().ok_or(())?;\n",
                "        match self {{\n",
                "{}",
                "        }}\n",
                "    }}\n",
                "}}\n",
            ),
            self.name,
            self.list.rs_variants(),
            self.name,
            ComplexDataType::min_len(self),
            ComplexDataType::max_len(self),
            self.list.rs_read(self.name),
            self.list.rs_write(self.name),
        )
    }

    fn hh(&self) -> String {
        format!(
            concat!(
                "class {}Reader {{\n",
                "public:\n",
                "    bool isValid(const u8 *buffer, u32 size, u32 &offset);\n",
                "    void read(const u8 *buffer, u32 &offset);\n",
                "\n",
                "private:\n",
                "{}",
                "}};\n",
                "\n",
                "class {}Writer {{\n",
                "public:\n",
                "{}\n",
                "    virtual bool write(u8 *buffer, u32 size, u32 &offset) = 0;\n",
                "}};\n",
                "{}",
            ),
            self.name,
            self.list.hh_read_delegates(),
            self.name,
            self.list.hh_writer_decls(),
            self.list.hh_writer_defs(self.name),
        )
    }

    fn cc(&self) -> String {
        format!(
            concat!(
                "bool {}Reader::isValid(const u8 *buffer, u32 size, u32 &offset) {{\n",
                "    if (offset + 1 > size) {{\n",
                "        return false;\n",
                "    }}\n",
                "    u8 discriminant = buffer[offset++];\n",
                "    switch (discriminant) {{\n",
                "{}",
                "    default:\n",
                "        return false;\n",
                "    }}\n",
                "}}\n",
                "\n",
                "void {}Reader::read(const u8 *buffer, u32 &offset) {{\n",
                "    u8 discriminant = buffer[offset++];\n",
                "    switch (discriminant) {{\n",
                "{}",
                "    }}\n",
                "}}\n",
                "{}",
            ),
            self.name,
            self.list.cc_is_valid(),
            self.name,
            self.list.cc_read(),
            self.list.cc_writers(self.name),
        )
    }
}
