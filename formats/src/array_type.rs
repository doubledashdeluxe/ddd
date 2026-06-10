use crate::data_type::DataType;
use crate::str_ext::StrExt;

pub enum ArrayType<T: DataType> {
    Fixed { data_type: T, len: usize },
    Variable { data_type: T, min_len: usize, max_len: usize },
}

impl<T: DataType> ArrayType<T> {
    pub fn new(data_type: T, min_len: usize, max_len: usize) -> Self {
        if min_len == max_len {
            Self::Fixed { data_type, len: min_len }
        } else {
            assert!(min_len < max_len);
            u8::try_from(max_len).unwrap();
            Self::Variable { data_type, min_len, max_len }
        }
    }
}

impl<T: DataType> DataType for ArrayType<T> {
    fn min_len(&self) -> usize {
        match self {
            Self::Fixed { data_type, len } => data_type.min_len() * len,
            Self::Variable { data_type, min_len, .. } => 1 + data_type.min_len() * min_len,
        }
    }

    fn max_len(&self) -> usize {
        match self {
            Self::Fixed { data_type, len } => data_type.max_len() * len,
            Self::Variable { data_type, max_len, .. } => 1 + data_type.max_len() * max_len,
        }
    }

    fn rs_name(&self) -> String {
        match self {
            Self::Fixed { data_type, len } => format!("[{}; {}]", data_type.rs_name(), len),
            Self::Variable { data_type, max_len, .. } => {
                format!("heapless::Vec<{}, {}>", data_type.rs_name(), max_len)
            }
        }
    }

    fn rs_read(&self, name: &str) -> String {
        match self {
            Self::Fixed { data_type, len } => {
                format!(
                    concat!(
                        "let mut {}: heapless::Vec<{}, {}> = heapless::Vec::new();\n",
                        "let buf = (0..{}).try_fold(buf, |buf, _| {{\n",
                        "    {}\n",
                        "    {}.push({}_element).unwrap();\n",
                        "    Ok(buf)\n",
                        "}})?;\n",
                        "let {} = {}.into_array().unwrap();",
                    ),
                    name,
                    data_type.rs_name(),
                    len,
                    len,
                    data_type.rs_read(&format!("{name}_element")).replace('\n', "\n    "),
                    name,
                    name,
                    name,
                    name,
                )
            }
            Self::Variable { data_type, min_len, max_len } => {
                let len_check = if *min_len == 0 {
                    format!("*{}_len > {}", name, *max_len)
                } else {
                    format!("*{name}_len < {min_len} || *{name}_len > {max_len}")
                };
                format!(
                    concat!(
                        "let ({}_len, buf) = buf.split_first().ok_or(())?;\n",
                        "if {} {{\n",
                        "    return Err(());\n",
                        "}}\n",
                        "let mut {} = heapless::Vec::new();\n",
                        "let buf = (0..*{}_len).try_fold(buf, |buf, _| {{\n",
                        "    {}\n",
                        "    {}.push({}_element).unwrap();\n",
                        "    Ok(buf)\n",
                        "}})?;",
                    ),
                    name,
                    len_check,
                    name,
                    name,
                    data_type.rs_read(&format!("{name}_element")).replace('\n', "\n    "),
                    name,
                    name,
                )
            }
        }
    }

    fn rs_write(&self, name: &str) -> String {
        match self {
            Self::Fixed { data_type, .. } => {
                format!(
                    concat!(
                        "let buf = {}.iter().try_fold(buf, |buf, {}_element| {{\n",
                        "    {}\n",
                        "    Ok(buf)\n",
                        "}})?;",
                    ),
                    name,
                    name,
                    data_type.rs_write(&format!("{name}_element")).replace('\n', "\n    "),
                )
            }
            Self::Variable { data_type, min_len, max_len } => {
                let len_check = if *min_len == 0 {
                    format!("{name}.len() > {max_len}")
                } else if *min_len == 1 {
                    format!("{name}.is_empty() || {name}.len() > {max_len}")
                } else {
                    format!("{}.len() < {} || {}.len() > {}", name, *min_len, name, *max_len)
                };
                format!(
                    concat!(
                        "if {} {{\n",
                        "    return Err(());\n",
                        "}}\n",
                        "let ({}_len, buf) = buf.split_first_mut().ok_or(())?;\n",
                        "*{}_len = {}.len() as u8;\n",
                        "let buf = {}.iter().try_fold(buf, |buf, {}_element| {{\n",
                        "    {}\n",
                        "    Ok(buf)\n",
                        "}})?;",
                    ),
                    len_check,
                    name,
                    name,
                    name,
                    name,
                    name,
                    data_type.rs_write(&format!("{name}_element")).replace('\n', "\n    "),
                )
            }
        }
    }

    fn hh_read_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        match self {
            Self::Fixed { data_type, .. } => data_type.hh_read_delegate(name, array_indices.next()),
            Self::Variable { data_type, .. } => {
                format!(
                    concat!(
                        "    virtual bool is{}CountValid({}u32 {}Count) = 0;\n",
                        "    virtual void set{}Count({}u32 {}Count) = 0;\n",
                        "{}",
                    ),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.typed_args(true),
                    name.to_ascii_camel_case(),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.typed_args(true),
                    name.to_ascii_camel_case(),
                    data_type.hh_read_delegate(name, array_indices.next()),
                )
            }
        }
    }

    fn hh_write_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        match self {
            Self::Fixed { data_type, .. } => {
                data_type.hh_write_delegate(name, array_indices.next())
            }
            Self::Variable { data_type, .. } => {
                format!(
                    concat!("    virtual u32 get{}Count({}) = 0;\n", "{}"),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.typed_args(false),
                    data_type.hh_write_delegate(name, array_indices.next()),
                )
            }
        }
    }

    fn cc_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
        match self {
            Self::Fixed { data_type, len } => {
                format!(
                    concat!("    for (u32 {} = 0; {} < {}; {}++) {{\n", "    {}\n", "    }}"),
                    array_indices.index(),
                    array_indices.index(),
                    len,
                    array_indices.index(),
                    data_type.cc_is_valid(name, array_indices.next()).replace('\n', "\n    "),
                )
            }
            Self::Variable { data_type, min_len, max_len } => {
                format!(
                    concat!(
                        "    if (offset + 1 > size) {{\n",
                        "        return false;\n",
                        "    }}\n",
                        "    u32 {} = buffer[offset++];\n",
                        "    if ({} < {} || {} > {}) {{\n",
                        "        return false;\n",
                        "    }}\n",
                        "    if (!is{}CountValid({}{})) {{\n",
                        "        return false;\n",
                        "    }}\n",
                        "    for (u32 {} = 0; {} < {}; {}++) {{\n",
                        "    {}\n",
                        "    }}",
                    ),
                    array_indices.count(),
                    array_indices.count(),
                    min_len,
                    array_indices.count(),
                    max_len,
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.untyped_args(true),
                    array_indices.count(),
                    array_indices.index(),
                    array_indices.index(),
                    array_indices.count(),
                    array_indices.index(),
                    data_type.cc_is_valid(name, array_indices.next()).replace('\n', "\n    "),
                )
            }
        }
    }

    fn cc_read(&self, name: &str, array_indices: ArrayIndices) -> String {
        match self {
            Self::Fixed { data_type, len } => {
                format!(
                    concat!("    for (u32 {} = 0; {} < {}; {}++) {{\n", "    {}\n", "    }}"),
                    array_indices.index(),
                    array_indices.index(),
                    len,
                    array_indices.index(),
                    data_type.cc_read(name, array_indices.next()).replace('\n', "\n    "),
                )
            }
            Self::Variable { data_type, .. } => {
                format!(
                    concat!(
                        "    u32 {} = buffer[offset++];\n",
                        "    set{}Count({}{});\n",
                        "    for (u32 {} = 0; {} < {}; {}++) {{\n",
                        "    {}\n",
                        "    }}",
                    ),
                    array_indices.count(),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.untyped_args(true),
                    array_indices.count(),
                    array_indices.index(),
                    array_indices.index(),
                    array_indices.count(),
                    array_indices.index(),
                    data_type.cc_read(name, array_indices.next()).replace('\n', "\n    "),
                )
            }
        }
    }

    fn cc_write(&self, name: &str, array_indices: ArrayIndices) -> String {
        match self {
            Self::Fixed { data_type, len } => {
                format!(
                    concat!("    for (u32 {} = 0; {} < {}; {}++) {{\n", "    {}\n", "    }}"),
                    array_indices.index(),
                    array_indices.index(),
                    len,
                    array_indices.index(),
                    data_type.cc_write(name, array_indices.next()).replace('\n', "\n    "),
                )
            }
            Self::Variable { data_type, min_len, max_len } => {
                format!(
                    concat!(
                        "    if (offset + 1 > size) {{\n",
                        "        return false;\n",
                        "    }}\n",
                        "    u32 {} = get{}Count({});\n",
                        "    if ({} < {} || {} > {}) {{\n",
                        "        return false;\n",
                        "    }}\n",
                        "    buffer[offset++] = {};\n",
                        "    for (u32 {} = 0; {} < {}; {}++) {{\n",
                        "    {}\n",
                        "    }}",
                    ),
                    array_indices.count(),
                    name.to_ascii_camel_case().to_ascii_sentence_case(),
                    array_indices.untyped_args(false),
                    array_indices.count(),
                    min_len,
                    array_indices.count(),
                    max_len,
                    array_indices.count(),
                    array_indices.index(),
                    array_indices.index(),
                    array_indices.count(),
                    array_indices.index(),
                    data_type.cc_write(name, array_indices.next()).replace('\n', "\n    "),
                )
            }
        }
    }
}

pub struct ArrayIndices<'a> {
    name: &'a str,
    count: usize,
}

impl<'a> ArrayIndices<'a> {
    pub const fn new(name: &'a str) -> Self {
        ArrayIndices { name, count: 0 }
    }

    pub const fn delegate_suffix(&self) -> &'static str {
        match self.count {
            0 => "",
            _ => "Element",
        }
    }

    pub fn typed_args(&self, trailing_comma: bool) -> String {
        let mut args: Vec<_> = (0..self.count).map(|i| format!("u32 i{i}")).collect();
        if trailing_comma {
            args.push(String::new());
        }
        args.join(", ")
    }

    pub fn untyped_args(&self, trailing_comma: bool) -> String {
        let mut args: Vec<_> = (0..self.count).map(|i| format!("i{i}")).collect();
        if trailing_comma {
            args.push(String::new());
        }
        args.join(", ")
    }

    fn count(&self) -> String {
        format!("{}Count{}", self.name.to_ascii_camel_case(), self.count)
    }

    fn index(&self) -> String {
        format!("i{}", self.count)
    }

    const fn next(self) -> Self {
        ArrayIndices { name: self.name, count: self.count + 1 }
    }
}
