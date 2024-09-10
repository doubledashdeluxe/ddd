pub trait StrExt {
    fn to_ascii_camel_case(&self) -> String;
    fn to_ascii_sentence_case(&self) -> String;
    fn to_ascii_snake_case(&self) -> String;
}

impl StrExt for str {
    fn to_ascii_camel_case(&self) -> String {
        let mut s = "".to_owned();
        let mut chars = self.chars();
        let Some(c) = chars.next() else {
            return s;
        };
        let mut pc = c.to_ascii_lowercase();
        for mut c in chars {
            if pc == '_' {
                c.make_ascii_uppercase();
            } else {
                s.push(pc);
            }
            pc = c;
        }
        s.push(pc);
        s
    }

    fn to_ascii_sentence_case(&self) -> String {
        let mut s = "".to_owned();
        let mut chars = self.chars();
        let Some(c) = chars.next() else {
            return s;
        };
        s.push(c.to_ascii_uppercase());
        s.extend(chars);
        s
    }

    fn to_ascii_snake_case(&self) -> String {
        let mut s = "".to_owned();
        let mut chars = self.chars();
        let Some(c) = chars.next() else {
            return s;
        };
        let mut pc = c;
        for c in chars {
            s.push(pc.to_ascii_lowercase());
            if pc.is_ascii_lowercase() && c.is_ascii_uppercase() {
                s.push('_');
            }
            pc = c;
        }
        s.push(pc.to_ascii_lowercase());
        s
    }
}

#[cfg(test)]
mod tests {
    use crate::str_ext::StrExt;

    #[test]
    fn test_to_ascii_camel_case() {
        assert_eq!("camelCase".to_ascii_camel_case(), "camelCase");
        assert_eq!("snake_case".to_ascii_camel_case(), "snakeCase");
    }

    #[test]
    fn test_to_ascii_sentence_case() {
        assert_eq!("camelCase".to_ascii_sentence_case(), "CamelCase");
        assert_eq!("snake_case".to_ascii_sentence_case(), "Snake_case");
    }

    #[test]
    fn test_to_ascii_snake_case() {
        assert_eq!("camelCase".to_ascii_snake_case(), "camel_case");
        assert_eq!("snake_case".to_ascii_snake_case(), "snake_case");
    }
}
