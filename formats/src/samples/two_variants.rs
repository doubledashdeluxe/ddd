#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum TwoVariants {
    First(heapless::Vec<u32, 3>),
    Second(()),
}

impl TwoVariants {
    const MIN_LEN: usize = 1;
    const MAX_LEN: usize = 14;

    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {
        let (discriminant, buf) = buf.split_first().ok_or(())?;
        match discriminant {
            0 => {
                let (first_len, buf) = buf.split_first().ok_or(())?;
                if *first_len < 1 || *first_len > 3 {
                    return Err(());
                }
                let mut first = heapless::Vec::new();
                let buf = (0..*first_len).try_fold(buf, |buf, _| {
                    let (first_element_buf, buf) = buf.split_first_chunk().ok_or(())?;
                    let first_element = u32::from_be_bytes(*first_element_buf);
                    first.push(first_element).unwrap();
                    Ok(buf)
                })?;
                Ok((Self::First(first), buf))
            }
            1 => {
                let second = ();
                Ok((Self::Second(second), buf))
            }
            _ => Err(()),
        }
    }

    pub fn write<'a>(&self, buf: &'a mut [u8]) -> Result<&'a mut [u8], ()> {
        let (discriminant, buf) = buf.split_first_mut().ok_or(())?;
        match self {
            Self::First(first) => {
                *discriminant = 0;
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
                Ok(buf)
            }
            Self::Second(second) => {
                *discriminant = 1;
                let () = second;
                Ok(buf)
            }
        }
    }
}
