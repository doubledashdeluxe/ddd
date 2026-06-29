use crate::array_type::ArrayIndices;
use crate::data_type::DataType;
use crate::str_ext::StrExt;

pub trait ComplexDataType: DataType {
    fn min_len(&self) -> usize;
    fn max_len(&self) -> usize;
    fn name(&self) -> &'static str;
    fn rs(&self) -> String;
    fn hh(&self) -> String;

    fn hh_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "        AssertType2<{}Reader<void> *(D::*)({}), {}Reader<D> *(D::*)({})>(&D::{}{}Reader);\n",
                "        if (!d->{}{}Reader({}) || !d->{}{}Reader({})->isValid(buffer, size, offset)) {{\n",
                "            return false;\n",
                "        }}",
            ),
            self.name(),
            array_indices.arg_types(false),
            self.name(),
            array_indices.arg_types(false),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }

    fn hh_read(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "        AssertType2<{}Reader<void> *(D::*)({}), {}Reader<D> *(D::*)({})>(&D::{}{}Reader);\n",
                "        d->{}{}Reader({})->read(buffer, offset);",
            ),
            self.name(),
            array_indices.arg_types(false),
            self.name(),
            array_indices.arg_types(false),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }

    fn hh_write(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "        AssertType<{}Writer<D> &(D::*)({})>(&D::{}{}Writer);\n",
                "        if (!d->{}{}Writer({}).write(buffer, size, offset)) {{\n",
                "            return false;\n",
                "        }}",
            ),
            self.name(),
            array_indices.arg_types(false),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }
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
        format!("let buf = {name}.write(buf)?;")
    }

    fn hh_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
        ComplexDataType::hh_is_valid(self, name, array_indices)
    }

    fn hh_read(&self, name: &str, array_indices: ArrayIndices) -> String {
        ComplexDataType::hh_read(self, name, array_indices)
    }

    fn hh_write(&self, name: &str, array_indices: ArrayIndices) -> String {
        ComplexDataType::hh_write(self, name, array_indices)
    }
}
