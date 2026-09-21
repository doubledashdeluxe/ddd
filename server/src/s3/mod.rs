use std::fmt::Write;

use anyhow::Result;
use jiff::Timestamp;
use jiff::fmt::rfc2822::DateTimePrinter;
use orion::hazardous::hash::sha2::sha256::Sha256;
use ureq::Agent;
use ureq::http::Method;
use ureq::http::header;
use ureq::http::uri::{Scheme, Uri};

use crate::credential::Credential;
use crate::http;
use crate::s3::hex_str::HexStr;
use crate::s3::request_builder::RequestBuilder;
use crate::s3::signer::Signer;

mod hex_str;
mod request_builder;
mod signer;

#[derive(Debug)]
pub struct S3 {
    agent: Agent,
    signer: Signer,
    max_age: String,
    dt: String,
}

impl S3 {
    pub fn put_object(
        &mut self,
        host: &str,
        region: &str,
        access_key: &Credential,
        secret_key: &Credential,
        max_age: u64,
        content_type: &str,
        path: &str,
        data: impl AsRef<[u8]>,
    ) -> Result<()> {
        self.signer.clear();
        self.dt.clear();
        self.max_age.clear();
        write!(&mut self.max_age, "public, max-age={max_age}, immutable")?;
        let digest = Sha256::digest(data.as_ref())?;
        let mut hashed_payload = HexStr::from(&digest);
        let uri =
            Uri::builder().scheme(Scheme::HTTPS).authority(host).path_and_query(path).build()?;
        let ts = Timestamp::now();
        DateTimePrinter::new().print_timestamp_rfc9110(&ts, &mut self.dt)?;
        let request = RequestBuilder::new(&mut self.signer)
            .method(Method::PUT)
            .uri(uri)
            .header(header::CACHE_CONTROL, &self.max_age)
            .header(header::CONTENT_TYPE, content_type)
            .header(header::DATE, &self.dt)
            .header(header::HOST, host)
            .header("x-amz-content-sha256", hashed_payload.encode()?)
            .sign(data.as_ref(), ts, region, access_key, secret_key, true)?;
        self.agent.run(request)?;
        Ok(())
    }
}

impl Default for S3 {
    fn default() -> Self {
        Self {
            agent: http::agent(),
            signer: Signer::default(),
            max_age: String::default(),
            dt: String::default(),
        }
    }
}
