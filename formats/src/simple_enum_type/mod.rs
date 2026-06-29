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
    pub const fn new(name: &'static str) -> Self {
        Self { name, list: () }
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
                "#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]\n",
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
            self.list.rs_read(),
            self.list.rs_write(),
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

    fn hh_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "        if (offset + 1 > size) {{\n",
                "            return false;\n",
                "        }}\n",
                "        u8 {} = buffer[offset++];\n",
                "        switch ({}) {{\n",
                "{}",
                "            AssertType<bool (D::*)({}u8)>(&D::is{}{}Valid);\n",
                "            if (!d->is{}{}Valid({}{})) {{\n",
                "                return false;\n",
                "            }}\n",
                "            break;\n",
                "        default:\n",
                "            return false;\n",
                "        }}",
            ),
            name.to_ascii_camel_case(),
            name.to_ascii_camel_case(),
            self.list.hh_cases(),
            array_indices.arg_types(true),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(true),
            name.to_ascii_camel_case(),
        )
    }

    fn hh_read(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "        u8 {} = buffer[offset++];\n",
                "        AssertType<void (D::*)({}u8)>(&D::set{}{});\n",
                "        d->set{}{}({}{});"
            ),
            name.to_ascii_camel_case(),
            array_indices.arg_types(true),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(true),
            name.to_ascii_camel_case(),
        )
    }

    fn hh_write(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "        if (offset + 1 > size) {{\n",
                "            return false;\n",
                "        }}\n",
                "        AssertType<u8 (D::*)({})>(&D::get{}{});\n",
                "        u8 {} = d->get{}{}({});\n",
                "        switch ({}) {{\n",
                "{}",
                "            buffer[offset++] = {};\n",
                "            break;\n",
                "        default:\n",
                "            return false;\n",
                "        }}",
            ),
            array_indices.arg_types(false),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
            name.to_ascii_camel_case(),
            self.list.hh_cases(),
            name.to_ascii_camel_case(),
        )
    }
}
