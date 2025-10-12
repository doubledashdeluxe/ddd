#[derive(Clone, Debug)]
pub enum OneVariant {
    First(u32),
}

impl OneVariant {
    const MIN_LEN: usize = 5;
    const MAX_LEN: usize = 5;

    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {
        let (discriminant, buf) = buf.split_first().ok_or(())?;
        match discriminant {
            0 => {
                let (first_buf, buf) = buf.split_first_chunk().ok_or(())?;
                let first = u32::from_be_bytes(*first_buf);
                Ok((OneVariant::First(first), buf))
            }
            _ => Err(()),
        }
    }

    pub fn write<'a>(&self, buf: &'a mut [u8]) -> Result<&'a mut [u8], ()> {
        let (discriminant, buf) = buf.split_first_mut().ok_or(())?;
        match self {
            OneVariant::First(first) => {
                *discriminant = 0;
                let (first_buf, buf) = buf.split_first_chunk_mut().ok_or(())?;
                *first_buf = first.to_be_bytes();
                Ok(buf)
            }
        }
    }
}
