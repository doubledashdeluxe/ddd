use ct_codecs::{Base64UrlSafe, Decoder, Encoder};
use serde::de::Error as _;
use serde::ser::Error as _;
use serde::{Deserialize, Deserializer, Serialize, Serializer};

pub fn serialize<S: Serializer, const N: usize>(
    arr: &[u8; N],
    serializer: S,
) -> Result<S::Ok, S::Error> {
    let mut s = [[0; 2]; N];
    let s = Base64UrlSafe::encode_to_str(s.as_flattened_mut(), arr).map_err(S::Error::custom)?;
    s.serialize(serializer)
}

pub fn deserialize<'de, D: Deserializer<'de>, const N: usize>(
    deserializer: D,
) -> Result<[u8; N], D::Error> {
    let s = <&str>::deserialize(deserializer)?;
    let mut arr = [0; N];
    Base64UrlSafe::decode(&mut arr, s, None).map_err(D::Error::custom)?;
    Ok(arr)
}
