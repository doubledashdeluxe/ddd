use crate::array_type::ArrayIndices;
use crate::data_type::DataType;
use crate::str_ext::StrExt;

pub trait ComplexDataType: DataType {
    fn min_len(&self) -> usize;
    fn max_len(&self) -> usize;
    fn name(&self) -> &'static str;
    fn rs(&self) -> String;
    fn hh(&self) -> String;
    fn cc(&self) -> String;
}

impl<T: ComplexDataType> DataType for T {
    fn min_len(&self) -> usize {
        ComplexDataType::min_len(self)
    }

    fn max_len(&self) -> usize {
        ComplexDataType::max_len(self)
    }

    fn rs_name(&self) -> String {
        self.name().to_owned()
    }

    fn rs_read(&self, name: &str) -> String {
        format!("let ({}, buf) = {}::read(buf)?;", name, self.name())
    }

    fn rs_write(&self, name: &str) -> String {
        format!("let buf = {}.write(buf)?;", name)
    }

    fn hh_read_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            "    virtual {}Reader *{}{}Reader({}) = 0;\n",
            self.name(),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.typed_args(false),
        )
    }

    fn hh_write_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            "    virtual {}Writer &{}{}Writer({}) = 0;\n",
            self.name(),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.typed_args(false),
        )
    }

    fn cc_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "    if (!{}{}Reader({}) || !{}{}Reader({})->isValid(buffer, size, offset)) {{\n",
                "        return false;\n",
                "    }}",
            ),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }

    fn cc_read(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            "    {}{}Reader({})->read(buffer, offset);",
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }

    fn cc_write(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "    if (!{}{}Writer({}).write(buffer, size, offset)) {{\n",
                "        return false;\n",
                "    }}",
            ),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }
}
