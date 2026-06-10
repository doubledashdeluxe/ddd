#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct TwoFields {
    pub first: [u32; 3],
    pub second: (),
}

impl TwoFields {
    const MIN_LEN: usize = 12;
    const MAX_LEN: usize = 12;

    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {
        let mut first: heapless::Vec<u32, 3> = heapless::Vec::new();
        let buf = (0..3).try_fold(buf, |buf, _| {
            let (first_element_buf, buf) = buf.split_first_chunk().ok_or(())?;
            let first_element = u32::from_be_bytes(*first_element_buf);
            first.push(first_element).unwrap();
            Ok(buf)
        })?;
        let first = first.into_array().unwrap();
        let second = ();
        #[rustfmt::skip]
        let two_fields = Self {
            first,
            second,
        };
        Ok((two_fields, buf))
    }

    pub fn write<'a>(&self, buf: &'a mut [u8]) -> Result<&'a mut [u8], ()> {
        #[rustfmt::skip]
        let Self {
            first,
            second,
        } = self;
        let buf = first.iter().try_fold(buf, |buf, first_element| {
            let (first_element_buf, buf) = buf.split_first_chunk_mut().ok_or(())?;
            *first_element_buf = first_element.to_be_bytes();
            Ok(buf)
        })?;
        let () = second;
        Ok(buf)
    }
}
