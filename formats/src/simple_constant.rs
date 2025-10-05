use crate::constant::Constant;
use crate::str_ext::StrExt;

pub struct SimpleConstant<T> {
    name: &'static str,
    value: T,
}

impl<T> SimpleConstant<T>
where
    SimpleConstant<T>: Constant,
{
    pub fn new(name: &'static str, value: T) -> SimpleConstant<T> {
        SimpleConstant { name, value }
    }
}

macro_rules! impl_simple_constant {
    ($t:ty, $ct:expr) => {
        impl Constant for SimpleConstant<$t> {
            fn rs(&self) -> String {
                format!("pub const {}: {} = {};\n", self.name, stringify!($t), self.value)
            }

            fn hh(&self) -> String {
                format!(
                    "const {} {} = {};\n",
                    $ct,
                    self.name.to_ascii_lowercase().to_ascii_camel_case().to_ascii_sentence_case(),
                    self.value
                )
            }
        }
    };
}

impl_simple_constant!(i8, "s8");
impl_simple_constant!(i16, "s16");
impl_simple_constant!(i32, "s32");
impl_simple_constant!(i64, "s64");
impl_simple_constant!(u8, "u8");
impl_simple_constant!(u16, "u16");
impl_simple_constant!(u32, "u32");
impl_simple_constant!(u64, "u64");
