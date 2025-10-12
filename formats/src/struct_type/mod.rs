use crate::complex_data_type::ComplexDataType;
use crate::data_type::DataType;
use crate::str_ext::StrExt;
use crate::struct_type::field::Field;
use crate::struct_type::field_list::FieldList;

mod field;
mod field_list;

pub struct StructType<L: FieldList> {
    name: &'static str,
    list: L,
}

impl StructType<()> {
    pub fn new(name: &'static str) -> StructType<()> {
        StructType { name, list: () }
    }
}

impl<L: FieldList> StructType<L> {
    pub fn with_field<T: DataType>(
        self,
        name: &'static str,
        data_type: T,
    ) -> StructType<(L, Field<T>)> {
        StructType { name: self.name, list: (self.list, Field::new(name, data_type)) }
    }
}

impl<L: FieldList> ComplexDataType for StructType<L> {
    fn min_len(&self) -> usize {
        self.list.min_len()
    }

    fn max_len(&self) -> usize {
        self.list.max_len()
    }

    fn name(&self) -> &'static str {
        self.name
    }

    fn rs(&self) -> String {
        format!(
            concat!(
                "#[derive(Clone, Debug)]\n",
                "pub struct {} {{\n",
                "{}}}\n",
                "\n",
                "impl {} {{\n",
                "    const MIN_LEN: usize = {};\n",
                "    const MAX_LEN: usize = {};\n",
                "\n",
                "    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {{\n",
                "{}",
                "        #[rustfmt::skip]\n",
                "        let {} = {} {{\n",
                "{}",
                "        }};\n",
                "        Ok(({}, buf))\n",
                "    }}\n",
                "\n",
                "    pub fn write<'a>(&self, buf: &'a mut [u8]) -> Result<&'a mut [u8], ()> {{\n",
                "        #[rustfmt::skip]\n",
                "        let {} {{\n",
                "{}",
                "        }} = self;\n",
                "{}",
                "        Ok(buf)\n",
                "    }}\n",
                "}}\n",
            ),
            self.name,
            self.list.rs_fields(),
            self.name,
            ComplexDataType::min_len(self),
            ComplexDataType::max_len(self),
            self.list.rs_read(),
            self.name.to_ascii_snake_case(),
            self.name,
            self.list.rs_field_names(),
            self.name.to_ascii_snake_case(),
            self.name,
            self.list.rs_field_names(),
            self.list.rs_write(),
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
                "    bool write(u8 *buffer, u32 size, u32 &offset);\n",
                "\n",
                "private:\n",
                "{}",
                "}};\n",
            ),
            self.name,
            self.list.hh_read_delegates(),
            self.name,
            self.list.hh_write_delegates(),
        )
    }

    fn cc(&self) -> String {
        format!(
            concat!(
                "bool {}Reader::isValid(const u8 *buffer, u32 size, u32 &offset) {{\n",
                "{}",
                "    return true;\n",
                "}}\n",
                "\n",
                "void {}Reader::read(const u8 *buffer, u32 &offset) {{\n",
                "{}",
                "}}\n",
                "\n",
                "bool {}Writer::write(u8 *buffer, u32 size, u32 &offset) {{\n",
                "{}",
                "    return true;\n",
                "}}\n",
            ),
            self.name,
            self.list.cc_is_valid(),
            self.name,
            self.list.cc_read(),
            self.name,
            self.list.cc_write(),
        )
    }
}
