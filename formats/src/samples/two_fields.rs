#[derive(Clone, Debug)]
pub struct TwoFields {
    pub first: Vec<u32>,
    pub second: (),
}

impl TwoFields {
    const MIN_LEN: usize = 5;
    const MAX_LEN: usize = 13;

    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {
        let (first_len, buf) = buf.split_first().ok_or(())?;
        if *first_len < 1 || *first_len > 3 {
            return Err(());
        }
        let mut first = Vec::with_capacity(*first_len as usize);
        let buf = (0..*first_len).try_fold(buf, |buf, _| {
            let (first_element_buf, buf) = buf.split_first_chunk().ok_or(())?;
            let first_element = u32::from_be_bytes(*first_element_buf);
            first.push(first_element);
            Ok(buf)
        })?;
        let second = ();
        #[rustfmt::skip]
        let two_fields = TwoFields {
            first,
            second,
        };
        Ok((two_fields, buf))
    }

    pub fn write<'a>(&self, buf: &'a mut [u8]) -> Result<&'a mut [u8], ()> {
        #[rustfmt::skip]
        let TwoFields {
            first,
            second,
        } = self;
        if first.is_empty() || first.len() > 3 {
            return Err(());
        }
        let (first_len, buf) = buf.split_first_mut().ok_or(())?;
        *first_len = first.len() as u8;
        let buf = first.iter().try_fold(buf, |buf, first_element| {
            let (first_element_buf, buf) = buf.split_first_chunk_mut().ok_or(())?;
            *first_element_buf = first_element.to_be_bytes();
            Ok(buf)
        })?;
        let _ = second;
        Ok(buf)
    }
}
