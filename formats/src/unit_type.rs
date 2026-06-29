use crate::array_type::ArrayIndices;
use crate::data_type::DataType;
use crate::str_ext::StrExt;

pub struct UnitType;

impl DataType for UnitType {
    fn min_len(&self) -> usize {
        0
    }

    fn max_len(&self) -> usize {
        0
    }

    fn rs_name(&self) -> String {
        "()".to_owned()
    }

    fn rs_read(&self, name: &str) -> String {
        format!("let {name} = ();")
    }

    fn rs_write(&self, name: &str) -> String {
        format!("let () = {name};")
    }

    fn hh_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "        AssertType<bool (D::*)({})>(&D::is{}{}Valid);\n",
                "        if (!d->is{}{}Valid({})) {{\n",
                "            return false;\n",
                "        }}",
            ),
            array_indices.arg_types(false),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }

    fn hh_read(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "        AssertType<void (D::*)({})>(&D::set{}{});\n",
                "        d->set{}{}({});",
            ),
            array_indices.arg_types(false),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }

    fn hh_write(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "        AssertType<void (D::*)({})>(&D::get{}{});\n",
                "        d->get{}{}({});",
            ),
            array_indices.arg_types(false),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }
}
