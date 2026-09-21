// Resources:
// - https://docs.aws.amazon.com/AmazonS3/latest/developerguide/sig-v4-header-based-auth.html

use std::fmt::{self, Debug, Formatter, Write as _};
use std::io::Write as _;

use anyhow::{Context, Result};
use jiff::Timestamp;
use jiff::fmt::rfc2822::DateTimePrinter;
use orion::hazardous::hash::sha2::sha256::Sha256;
use orion::hazardous::mac::hmac::sha256::{HmacSha256, Tag};
use percent_encoding::{AsciiSet, NON_ALPHANUMERIC};
use ureq::http::Request;
use ureq::http::header;

use crate::credential::Credential;
use crate::s3::hex_str::HexStr;

#[derive(Debug, Default)]
pub struct Signer {
    canonical_query_string: String,
    canonical_headers: Vec<u8>,
    signed_headers: String,
    string_to_sign: String,
    authorization: Authorization,
}

impl Signer {
    pub fn clear(&mut self) {
        self.canonical_query_string.clear();
        self.canonical_headers.clear();
        self.signed_headers.clear();
        self.string_to_sign.clear();
        self.authorization.0.clear();
    }

    pub fn header(&mut self, name: &str, value: &[u8]) {
        self.canonical_headers.extend(name.as_bytes().iter().map(u8::to_ascii_lowercase));
        self.canonical_headers.push(b':');
        self.canonical_headers.extend(value.trim_ascii());
        self.canonical_headers.push(b'\n');

        if !self.signed_headers.is_empty() {
            self.signed_headers.push(';');
        }
        self.signed_headers.extend(name.chars().map(|c| c.to_ascii_lowercase()));
    }

    pub fn sign(
        &mut self,
        request: &mut Request<&[u8]>,
        ts: Timestamp,
        region: &str,
        access_key: &Credential,
        secret_key: &Credential,
        rfc9110_date: bool,
    ) -> Result<()> {
        let hashed_payload = request
            .headers()
            .get("x-amz-content-sha256")
            .context("missing x-amz-content-sha256 header")?;

        let mut sha256 = Sha256::new();
        sha256.update(request.method().as_str().as_bytes())?;
        sha256.update(b"\n")?;
        for part in percent_encode(request.uri().path().as_bytes()) {
            sha256.update(part.as_bytes())?;
        }
        sha256.update(b"\n\n")?;
        sha256.update(&self.canonical_headers)?;
        sha256.update(b"\n")?;
        sha256.update(self.signed_headers.as_bytes())?;
        sha256.update(b"\n")?;
        sha256.update(hashed_payload.as_bytes())?;
        let digest = sha256.finalize()?;
        let mut hashed_request = HexStr::from(&digest);

        writeln!(&mut self.string_to_sign, "AWS4-HMAC-SHA256")?;
        if rfc9110_date {
            // AWS
            DateTimePrinter::new().print_timestamp_rfc9110(&ts, &mut self.string_to_sign)?;
            writeln!(&mut self.string_to_sign)?;
        } else {
            // AWS docs and OCI
            writeln!(&mut self.string_to_sign, "{}", ts.strftime("%Y%m%dT%H%M%SZ"))?;
        }
        writeln!(&mut self.string_to_sign, "{}/{region}/s3/aws4_request", ts.strftime("%Y%m%d"))?;
        write!(&mut self.string_to_sign, "{}", hashed_request.encode()?)?;

        let mut key = [0; 64];
        write!(key.as_mut_slice(), "AWS4{}", secret_key.0)?;
        let mut date = [0; 8];
        write!(date.as_mut_slice(), "{}", ts.strftime("%Y%m%d"))?;
        let key = hmac(key, date)?;
        let key = hmac(key.unprotected_as_ref(), region)?;
        let key = hmac(key.unprotected_as_ref(), "s3")?;
        let key = hmac(key.unprotected_as_ref(), "aws4_request")?;
        let tag = hmac(key.unprotected_as_ref(), &self.string_to_sign)?;
        let tag = tag.unprotected_as_ref();
        let mut signature = HexStr::from(&tag);

        write!(
            &mut self.authorization.0,
            "AWS4-HMAC-SHA256 Credential={}/{}/{}/s3/aws4_request,SignedHeaders={},Signature={}",
            access_key.0,
            ts.strftime("%Y%m%d"),
            region,
            self.signed_headers,
            signature.encode()?,
        )?;
        request
            .headers_mut()
            .try_append(header::AUTHORIZATION, self.authorization.0.as_str().try_into()?)?;

        Ok(())
    }
}

#[derive(Default)]
struct Authorization(String);

impl Debug for Authorization {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        f.debug_tuple("Authorization").finish_non_exhaustive()
    }
}

fn percent_encode(data: &[u8]) -> impl Iterator<Item = &str> {
    percent_encoding::percent_encode(data, &RESERVED)
}

const RESERVED: AsciiSet =
    NON_ALPHANUMERIC.remove(b'-').remove(b'.').remove(b'/').remove(b'_').remove(b'~');

fn hmac(key: impl AsRef<[u8]>, data: impl AsRef<[u8]>) -> Result<Tag> {
    Ok(HmacSha256::hmac(&key.as_ref().try_into()?, data.as_ref())?)
}
