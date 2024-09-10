use std::marker::PhantomData;

use crate::array_type::ArrayIndices;
use crate::data_type::DataType;
use crate::str_ext::StrExt;

pub struct SimpleDataType<T> {
    marker: PhantomData<T>,
}

impl<T> SimpleDataType<T>
where
    SimpleDataType<T>: DataType,
{
    pub fn new() -> SimpleDataType<T> {
        SimpleDataType { marker: PhantomData }
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

            fn hh_read_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
                format!(
                    concat!(
                        "    virtual bool is{}{}Valid({}{} {}{}) = 0;\n",
                        "    virtual void set{}{}({}{} {}{}) = 0;\n",
                    ),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    array_indices.typed_args(true),
                    $ct,
                    name.to_ascii_camel_case(),
                    array_indices.delegate_suffix(),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    array_indices.typed_args(true),
                    $ct,
                    name.to_ascii_camel_case(),
                    array_indices.delegate_suffix(),
                )
            }

            fn hh_write_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
                format!(
                    "    virtual {} get{}{}({}) = 0;\n",
                    $ct,
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    array_indices.typed_args(false),
                )
            }

            fn cc_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
                format!(
                    concat!(
                        "    if (offset + {} > size) {{\n",
                        "        return false;\n",
                        "    }}\n",
                        "    if (!is{}{}Valid({}Bytes::ReadBE<{}>(buffer, offset))) {{\n",
                        "        return false;\n",
                        "    }}\n",
                        "    offset += {};",
                    ),
                    $l,
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    array_indices.untyped_args(true),
                    $ct,
                    $l,
                )
            }

            fn cc_read(&self, name: &str, array_indices: ArrayIndices) -> String {
                format!(
                    concat!(
                        "    set{}{}({}Bytes::ReadBE<{}>(buffer, offset));\n",
                        "    offset += {};",
                    ),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.delegate_suffix(),
                    array_indices.untyped_args(true),
                    $ct,
                    $l,
                )
            }

            fn cc_write(&self, name: &str, array_indices: ArrayIndices) -> String {
                format!(
                    concat!(
                        "    if (offset + {} > size) {{\n",
                        "        return false;\n",
                        "    }}\n",
                        "    Bytes::WriteBE<{}>(buffer, offset, get{}{}({}));\n",
                        "    offset += {};",
                    ),
                    $l,
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
