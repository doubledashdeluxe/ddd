#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
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
            0 => Ok((First::A, buf)),
            1 => Ok((First::B, buf)),
            _ => Err(()),
        }
    }

    pub fn write<'a>(&self, buf: &'a mut [u8]) -> Result<&'a mut [u8], ()> {
        let (discriminant, buf) = buf.split_first_mut().ok_or(())?;
        *discriminant = match self {
            First::A => 0,
            First::B => 1,
        };
        Ok(buf)
    }
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum OneVariant {
    First(First),
}

impl OneVariant {
    const MIN_LEN: usize = 2;
    const MAX_LEN: usize = 2;

    pub fn read(buf: &[u8]) -> Result<(Self, &[u8]), ()> {
        let (discriminant, buf) = buf.split_first().ok_or(())?;
        match discriminant {
            0 => {
                let (first, buf) = First::read(buf)?;
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
                let buf = first.write(buf)?;
                Ok(buf)
            }
        }
    }
}
