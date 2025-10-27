pub struct Variant {
    name: &'static str,
}

impl Variant {
    pub fn new(name: &'static str) -> Variant {
        Variant { name }
    }

    pub fn name(&self) -> &'static str {
        self.name
    }
}
