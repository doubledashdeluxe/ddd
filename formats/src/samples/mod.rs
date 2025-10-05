#![allow(dead_code)]

mod one_constant;
mod one_field;
mod one_variant;
mod two_constants;
mod two_fields;
mod two_variants;

#[cfg(test)]
mod tests {
    use crate::array_type::ArrayType;
    use crate::enum_type::EnumType;
    use crate::format::Format;
    use crate::simple_constant::SimpleConstant;
    use crate::simple_data_type::SimpleDataType;
    use crate::struct_type::StructType;

    #[test]
    fn test_one_constant() {
        let first: SimpleConstant<u32> = SimpleConstant::new("FIRST", 1);
        let one_constant = Format::new("OneConstant").with_constant(first);
        assert_eq_multiline(&one_constant.rs(), include_str!("one_constant.rs"));
        assert_eq_multiline(&one_constant.hh(), include_str!("OneConstant.hh"));
        assert_eq_multiline(&one_constant.cc(), include_str!("OneConstant.cc"));
    }

    #[test]
    fn test_two_constants() {
        let first: SimpleConstant<u32> = SimpleConstant::new("FIRST", 1);
        let second: SimpleConstant<u8> = SimpleConstant::new("SECOND", 2);
        let two_constants = Format::new("TwoConstants").with_constant(first).with_constant(second);
        assert_eq_multiline(&two_constants.rs(), include_str!("two_constants.rs"));
        assert_eq_multiline(&two_constants.hh(), include_str!("TwoConstants.hh"));
        assert_eq_multiline(&two_constants.cc(), include_str!("TwoConstants.cc"));
    }

    #[test]
    fn test_one_variant() {
        let first: SimpleDataType<u32> = SimpleDataType::new();
        let one_variant = EnumType::new("OneVariant").with_variant("First", first);
        let one_variant = Format::new("OneVariant").with_type(one_variant);
        assert_eq_multiline(&one_variant.rs(), include_str!("one_variant.rs"));
        assert_eq_multiline(&one_variant.hh(), include_str!("OneVariant.hh"));
        assert_eq_multiline(&one_variant.cc(), include_str!("OneVariant.cc"));
    }

    #[test]
    fn test_two_variants() {
        let first_element: SimpleDataType<u32> = SimpleDataType::new();
        let first = ArrayType::new(first_element, 1, 3);
        let second: SimpleDataType<u8> = SimpleDataType::new();
        let two_variants = EnumType::new("TwoVariants")
            .with_variant("First", first)
            .with_variant("Second", second);
        let two_variants = Format::new("TwoVariants").with_type(two_variants);
        assert_eq_multiline(&two_variants.rs(), include_str!("two_variants.rs"));
        assert_eq_multiline(&two_variants.hh(), include_str!("TwoVariants.hh"));
        assert_eq_multiline(&two_variants.cc(), include_str!("TwoVariants.cc"));
    }

    #[test]
    fn test_one_field() {
        let first: SimpleDataType<u32> = SimpleDataType::new();
        let one_field = StructType::new("OneField").with_field("first", first);
        let one_field = Format::new("OneField").with_type(one_field);
        assert_eq_multiline(&one_field.rs(), include_str!("one_field.rs"));
        assert_eq_multiline(&one_field.hh(), include_str!("OneField.hh"));
        assert_eq_multiline(&one_field.cc(), include_str!("OneField.cc"));
    }

    #[test]
    fn test_two_fields() {
        let first_element: SimpleDataType<u32> = SimpleDataType::new();
        let first = ArrayType::new(first_element, 1, 3);
        let second: SimpleDataType<u8> = SimpleDataType::new();
        let two_fields =
            StructType::new("TwoFields").with_field("first", first).with_field("second", second);
        let two_fields = Format::new("TwoFields").with_type(two_fields);
        assert_eq_multiline(&two_fields.rs(), include_str!("two_fields.rs"));
        assert_eq_multiline(&two_fields.hh(), include_str!("TwoFields.hh"));
        assert_eq_multiline(&two_fields.cc(), include_str!("TwoFields.cc"));
    }

    fn assert_eq_multiline(l: &str, r: &str) {
        let (mut l, mut r) = (l.lines(), r.lines());
        for i in (1..).into_iter() {
            match (l.next(), r.next()) {
                (None, None) => return,
                (l, r) => assert_eq!((i, l), (i, r)),
            }
        }
    }
}
