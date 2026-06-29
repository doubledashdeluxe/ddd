use std::marker::PhantomData;

use crate::array_type::ArrayIndices;
use crate::data_type::DataType;
use crate::str_ext::StrExt;

pub struct SimpleDataType<T> {
    marker: PhantomData<T>,
}

impl<T> SimpleDataType<T>
where
    Self: DataType,
{
    pub const fn new() -> Self {
        Self { marker: PhantomData }
    }
}

macro_rules! impl_simple_data_type {
    ($t:ty, $l:expr, $ct:expr) => {
        impl DataType for SimpleDataType<$t> {
            fn min_len(&self) -> usize {
                $l
            }

            fn max_len(&self) -> usize {
                $l
            }

            fn rs_name(&self) -> String {
                stringify!($t).to_owned()
            }

            fn rs_read(&self, name: &str) -> String {
                format!(
                    concat!(
                        "let ({}_buf, buf) = buf.split_first_chunk().ok_or(())?;\n",
                        "let {} = {}::from_be_bytes(*{}_buf);",
                    ),
                    name,
                    name,
                    stringify!($t),
                    name,
                )
            }

            fn rs_write(&self, name: &str) -> String {
                format!(
                    concat!(
                        "let ({}_buf, buf) = buf.split_first_chunk_mut().ok_or(())?;\n",
                        "*{}_buf = {}.to_be_bytes();",
                    ),
                    name, name, name,
                )
            }

            fn hh_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
                format!(
                    concat!(
                        "        if (offset + {} > size) {{\n",
                        "            return false;\n",
                        "        }}\n",
                        "        AssertType<bool (D::*)({}{})>(&D::is{}{}Valid);\n",
                        "        if (!d->is{}{}Valid({}Bytes::ReadBE<{}>(buffer, offset))) {{\n",
                        "            return false;\n",
                        "        }}\n",
                        "        offset += {};",
                    ),
                    $l,
                    array_indices.arg_types(true),
                    $ct,
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    array_indices.untyped_args(true),
                    $ct,
                    $l,
                )
            }

            fn hh_read(&self, name: &str, array_indices: ArrayIndices) -> String {
                format!(
                    concat!(
                        "        AssertType<void (D::*)({}{})>(&D::set{}{});\n",
                        "        d->set{}{}({}Bytes::ReadBE<{}>(buffer, offset));\n",
                        "        offset += {};",
                    ),
                    array_indices.arg_types(true),
                    $ct,
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    array_indices.untyped_args(true),
                    $ct,
                    $l,
                )
            }

            fn hh_write(&self, name: &str, array_indices: ArrayIndices) -> String {
                format!(
                    concat!(
                        "        if (offset + {} > size) {{\n",
                        "            return false;\n",
                        "        }}\n",
                        "        AssertType<{} (D::*)({})>(&D::get{}{});\n",
                        "        Bytes::WriteBE<{}>(buffer, offset, d->get{}{}({}));\n",
                        "        offset += {};",
                    ),
                    $l,
                    $ct,
                    array_indices.arg_types(false),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    $ct,
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    array_indices.untyped_args(false),
                    $l,
                )
            }
        }
    };
}

impl_simple_data_type!(i8, 1, "s8");
impl_simple_data_type!(i16, 2, "s16");
impl_simple_data_type!(i32, 4, "s32");
impl_simple_data_type!(i64, 8, "s64");
impl_simple_data_type!(u8, 1, "u8");
impl_simple_data_type!(u16, 2, "u16");
impl_simple_data_type!(u32, 4, "u32");
impl_simple_data_type!(u64, 8, "u64");
