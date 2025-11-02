use crate::array_type::ArrayIndices;
use crate::complex_data_type::ComplexDataType;
use crate::simple_enum_type::variant::Variant;
use crate::simple_enum_type::variant_list::VariantList;
use crate::str_ext::StrExt;

mod variant;
mod variant_list;

pub struct SimpleEnumType<L: VariantList> {
    name: &'static str,
    list: L,
}

impl SimpleEnumType<()> {
    pub fn new(name: &'static str) -> SimpleEnumType<()> {
        SimpleEnumType { name, list: () }
    }
}

impl<L: VariantList> SimpleEnumType<L> {
    pub fn with_variant(self, name: &'static str) -> SimpleEnumType<(L, Variant)> {
        SimpleEnumType { name: self.name, list: (self.list, Variant::new(name)) }
    }
}

impl<L: VariantList> ComplexDataType for SimpleEnumType<L> {
    fn min_len(&self) -> usize {
        1
    }

    fn max_len(&self) -> usize {
        1
    }

    fn name(&self) -> &'static str {
        self.name
    }

    fn rs(&self) -> String {
        format!(
            concat!(
                "#[derive(Clone, Copy, Debug, PartialEq)]\n",
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
                "        *discriminant = match self {{\n",
                "{}",
                "        }};\n",
                "        Ok(buf)\n",
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
                "class {} {{\n",
                "public:\n",
                "    enum {{\n",
                "{}",
                "        Count = {},\n",
                "    }};\n",
                "\n",
                "private:\n",
                "    {}();\n",
                "}};\n",
            ),
            self.name,
            self.list.hh_variants(),
            L::count(),
            self.name,
        )
    }

    fn cc(&self) -> String {
        "".to_owned()
    }

    fn hh_read_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "    virtual bool is{}{}Valid({}u8 {}{}) = 0;\n",
                "    virtual void set{}{}({}u8 {}{}) = 0;\n",
            ),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.typed_args(true),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.typed_args(true),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
        )
    }

    fn hh_write_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            "    virtual u8 get{}{}({}) = 0;\n",
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.typed_args(false),
        )
    }

    fn cc_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "    if (offset + 1 > size) {{\n",
                "        return false;\n",
                "    }}\n",
                "    u8 {} = buffer[offset++];\n",
                "    switch ({}) {{\n",
                "{}",
                "        if (!is{}{}Valid({}{})) {{\n",
                "            return false;\n",
                "        }}\n",
                "        break;\n",
                "    default:\n",
                "        return false;\n",
                "    }}",
            ),
            name.to_ascii_camel_case(),
            name.to_ascii_camel_case(),
            self.list.cc_cases(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(true),
            name.to_ascii_camel_case(),
        )
    }

    fn cc_read(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!("    u8 {} = buffer[offset++];\n", "    set{}{}({}{});",),
            name.to_ascii_camel_case(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(true),
            name.to_ascii_camel_case(),
        )
    }

    fn cc_write(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "    if (offset + 1 > size) {{\n",
                "        return false;\n",
                "    }}\n",
                "    u8 {} = get{}{}({});\n",
                "    switch ({}) {{\n",
                "{}",
                "        buffer[offset++] = {};\n",
                "        break;\n",
                "    default:\n",
                "        return false;\n",
                "    }}",
            ),
            name.to_ascii_camel_case(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
            name.to_ascii_camel_case(),
            self.list.cc_cases(),
            name.to_ascii_camel_case(),
        )
    }
}
