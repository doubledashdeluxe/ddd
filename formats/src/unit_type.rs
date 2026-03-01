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

    fn hh_read_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "    virtual bool is{}{}Valid({}) = 0;\n",
                "    virtual void set{}{}({}) = 0;\n",
            ),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.typed_args(false),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.typed_args(false),
        )
    }

    fn hh_write_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            "    virtual void get{}{}({}) = 0;\n",
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.typed_args(false),
        )
    }

    fn cc_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!("    if (!is{}{}Valid({})) {{\n", "        return false;\n", "    }}"),
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }

    fn cc_read(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            "    set{}{}({});",
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }

    fn cc_write(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            "    get{}{}({});",
            name.to_ascii_camel_case().to_ascii_sentence_case(),
            array_indices.delegate_suffix(),
            array_indices.untyped_args(false),
        )
    }
}
