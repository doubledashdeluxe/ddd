use crate::data_type::DataType;
use crate::str_ext::StrExt;

pub struct ArrayType<T: DataType> {
    data_type: T,
    min_len: u8,
    max_len: u8,
}

impl<T: DataType> ArrayType<T> {
    pub fn new(data_type: T, min_len: u8, max_len: u8) -> ArrayType<T> {
        assert!(min_len <= max_len);
        ArrayType { data_type, min_len, max_len }
    }
}

impl<T: DataType> DataType for ArrayType<T> {
    fn min_len(&self) -> usize {
        1 + self.data_type.min_len() * self.min_len as usize
    }

    fn max_len(&self) -> usize {
        1 + self.data_type.max_len() * self.max_len as usize
    }

    fn rs_name(&self) -> String {
        format!("Vec<{}>", self.data_type.rs_name())
    }

    fn rs_read(&self, name: &str) -> String {
        format!(
            concat!(
                "let ({}_len, buf) = buf.split_first().ok_or(())?;\n",
                "if *{}_len < {} || *{}_len > {} {{\n",
                "    return Err(());\n",
                "}}\n",
                "let mut {} = Vec::with_capacity(*{}_len as usize);\n",
                "let buf = (0..*{}_len).try_fold(buf, |buf, _| {{\n",
                "    {}\n",
                "    {}.push({}_element);\n",
                "    Ok(buf)\n",
                "}})?;",
            ),
            name,
            name,
            self.min_len,
            name,
            self.max_len,
            name,
            name,
            name,
            self.data_type.rs_read(&format!("{}_element", name)).replace("\n", "\n    "),
            name,
            name,
        )
    }

    fn rs_write(&self, name: &str) -> String {
        format!(
            concat!(
                "if {}.len() < {} || {}.len() > {} {{\n",
                "    return Err(());\n",
                "}}\n",
                "let ({}_len, buf) = buf.split_first_mut().ok_or(())?;\n",
                "*{}_len = {}.len() as u8;\n",
                "let buf = {}.into_iter().try_fold(buf, |buf, {}_element| {{\n",
                "    {}\n",
                "    Ok(buf)\n",
                "}})?;",
            ),
            name,
            self.min_len,
            name,
            self.max_len,
            name,
            name,
            name,
            name,
            name,
            self.data_type.rs_write(&format!("{}_element", name)).replace("\n", "\n    "),
        )
    }

    fn hh_read_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "    virtual bool is{}CountValid({}u32 {}Count) = 0;\n",
                "    virtual void set{}Count({}u32 {}Count) = 0;\n",
                "{}",
            ),
            name,
            array_indices.typed_args(true),
            name.to_ascii_camel_case(),
            name,
            array_indices.typed_args(true),
            name.to_ascii_camel_case(),
            self.data_type.hh_read_delegate(name, array_indices.next()),
        )
    }

    fn hh_write_delegate(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!("    virtual u32 get{}Count({}) = 0;\n", "{}"),
            name,
            array_indices.typed_args(false),
            self.data_type.hh_write_delegate(name, array_indices.next()),
        )
    }

    fn cc_is_valid(&self, name: &str, array_indices: ArrayIndices) -> String {
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
            self.min_len,
            array_indices.count(),
            self.max_len,
            name,
            array_indices.untyped_args(true),
            array_indices.count(),
            array_indices.index(),
            array_indices.index(),
            array_indices.count(),
            array_indices.index(),
            self.data_type.cc_is_valid(name, array_indices.next()).replace("\n", "\n    "),
        )
    }

    fn cc_read(&self, name: &str, array_indices: ArrayIndices) -> String {
        format!(
            concat!(
                "    u32 {} = buffer[offset++];\n",
                "    set{}Count({}{});\n",
                "    for (u32 {} = 0; {} < {}; {}++) {{\n",
                "    {}\n",
                "    }}",
            ),
            array_indices.count(),
            name,
            array_indices.untyped_args(true),
            array_indices.count(),
            array_indices.index(),
            array_indices.index(),
            array_indices.count(),
            array_indices.index(),
            self.data_type.cc_read(name, array_indices.next()).replace("\n", "\n    "),
        )
    }

    fn cc_write(&self, name: &str, array_indices: ArrayIndices) -> String {
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
            name,
            array_indices.untyped_args(false),
            array_indices.count(),
            self.min_len,
            array_indices.count(),
            self.max_len,
            array_indices.count(),
            array_indices.index(),
            array_indices.index(),
            array_indices.count(),
            array_indices.index(),
            self.data_type.cc_write(name, array_indices.next()).replace("\n", "\n    "),
        )
    }
}

pub struct ArrayIndices {
    count: usize,
}

impl ArrayIndices {
    pub fn new() -> ArrayIndices {
        ArrayIndices { count: 0 }
    }

    pub fn delegate_suffix(&self) -> &'static str {
        match self.count {
            0 => "",
            _ => "Element",
        }
    }

    pub fn typed_args(&self, trailing_comma: bool) -> String {
        let mut args: Vec<_> = (0..self.count).map(|i| format!("u32 i{}", i)).collect();
        if trailing_comma {
            args.push("".to_owned());
        }
        args.join(", ")
    }

    pub fn untyped_args(&self, trailing_comma: bool) -> String {
        let mut args: Vec<_> = (0..self.count).map(|i| format!("i{}", i)).collect();
        if trailing_comma {
            args.push("".to_owned());
        }
        args.join(", ")
    }

    fn count(&self) -> String {
        format!("c{}", self.count)
    }

    fn index(&self) -> String {
        format!("i{}", self.count)
    }

    fn next(self) -> ArrayIndices {
        ArrayIndices { count: self.count + 1 }
    }
}
