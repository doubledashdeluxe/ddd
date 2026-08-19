#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq, serde::Serialize, serde::Deserialize)]
pub enum First {
    A,
    B,
}

impl First {
    const MIN_LEN: usize = 1;
    const MAX_LEN: usize = 1;

    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {
        let (discriminant, buf) = buf.split_first().ok_or(())?;
        match discriminant {
            0 => Ok((Self::A, buf)),
            1 => Ok((Self::B, buf)),
            _ => Err(()),
        }
    }

    pub fn write<'a>(&self, buf: &'a mut [u8]) -> Result<&'a mut [u8], ()> {
        let (discriminant, buf) = buf.split_first_mut().ok_or(())?;
        *discriminant = match self {
            Self::A => 0,
            Self::B => 1,
        };
        Ok(buf)
    }
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct OneField {
    pub first: First,
}

impl OneField {
    const MIN_LEN: usize = 1;
    const MAX_LEN: usize = 1;

    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {
        let (first, buf) = First::read(buf)?;
        #[rustfmt::skip]
        let one_field = Self {
            first,
        };
        Ok((one_field, buf))
    }

    pub fn write<'a>(&self, buf: &'a mut [u8]) -> Result<&'a mut [u8], ()> {
        #[rustfmt::skip]
        let Self {
            first,
        } = self;
        let buf = first.write(buf)?;
        Ok(buf)
    }
}
