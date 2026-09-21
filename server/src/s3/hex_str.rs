use anyhow::Result;
use ct_codecs::{Encoder, Hex};

pub struct HexStr<'a> {
    bin: &'a [u8],
    hex: [u8; 64],
}

impl<'a, B: AsRef<[u8]>> From<&'a B> for HexStr<'a> {
    fn from(bin: &'a B) -> Self {
        Self { bin: bin.as_ref(), hex: [0; _] }
    }
}

impl HexStr<'_> {
    pub fn encode(&mut self) -> Result<&str> {
        Ok(Hex::encode_to_str(&mut self.hex, self.bin)?)
    }
}
