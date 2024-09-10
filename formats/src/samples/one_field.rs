#[derive(Clone, Debug)]
pub struct OneField {
    pub first: u32,
}

impl OneField {
    const MIN_LEN: usize = 4;
    const MAX_LEN: usize = 4;

    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {
        let (first_buf, buf) = buf.split_first_chunk().ok_or(())?;
        let first = u32::from_be_bytes(*first_buf);
        #[rustfmt::skip]
        let one_field = OneField {
            first,
        };
        Ok((one_field, buf))
    }

    pub fn write(self, buf: &mut [u8]) -> Result<&mut [u8], ()> {
        #[rustfmt::skip]
        let OneField {
            first,
        } = self;
        let (first_buf, buf) = buf.split_first_chunk_mut().ok_or(())?;
        *first_buf = first.to_be_bytes();
        Ok(buf)
    }
}
