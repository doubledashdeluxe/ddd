use crate::complex_data_type::ComplexDataType;
use crate::simple_enum_type::SimpleEnumType;

pub fn mode_index() -> impl ComplexDataType {
    SimpleEnumType::new("ModeIndex")
        .with_variant("Versus")
        .with_variant("Balloon")
        .with_variant("Escape")
        .with_variant("Bomb")
        .with_variant("TimeAttack")
}
