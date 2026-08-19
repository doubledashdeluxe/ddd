use serde::{Deserialize, Deserializer, Serialize, Serializer};

#[derive(Clone, Copy, Debug)]
pub struct Name(pub [u8; 3]);

impl Serialize for Name {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        str::from_utf8(&self.0).unwrap_or("   ").serialize(serializer)
    }
}

impl<'de> Deserialize<'de> for Name {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        let name = <&str>::deserialize(deserializer)?;
        Ok(Self(name.as_bytes().try_into().unwrap_or(*b"   ")))
    }
}
