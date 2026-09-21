use anyhow::{Error, Result};
use jiff::Timestamp;
use ureq::http::{HeaderName, HeaderValue, Method, Request, Uri};

use crate::credential::Credential;
use crate::s3::signer::Signer;

pub struct RequestBuilder<'a> {
    signer: &'a mut Signer,
    request: Result<Request<()>>,
}

impl<'a> RequestBuilder<'a> {
    pub fn new(signer: &'a mut Signer) -> Self {
        Self { signer, request: Ok(Request::new(())) }
    }

    pub fn method<M: TryInto<Method>>(self, method: M) -> Self
    where
        Error: From<M::Error>,
    {
        self.and_then(|_, mut request| {
            *request.method_mut() = method.try_into()?;
            Ok(request)
        })
    }

    pub fn uri<U: TryInto<Uri>>(self, uri: U) -> Self
    where
        Error: From<U::Error>,
    {
        self.and_then(|_, mut request| {
            *request.uri_mut() = uri.try_into()?;
            Ok(request)
        })
    }

    pub fn header<N: TryInto<HeaderName>, V: TryInto<HeaderValue>>(self, name: N, value: V) -> Self
    where
        Error: From<N::Error> + From<V::Error>,
    {
        self.and_then(|signer, mut request| {
            let name = name.try_into()?;
            let value = value.try_into()?;
            signer.header(name.as_str(), value.as_bytes());
            request.headers_mut().try_append(name, value)?;
            Ok(request)
        })
    }

    fn and_then(self, f: impl FnOnce(&mut Signer, Request<()>) -> Result<Request<()>>) -> Self {
        Self { request: self.request.and_then(|request| f(self.signer, request)), ..self }
    }

    pub fn sign<'b>(
        self,
        body: &'b [u8],
        ts: Timestamp,
        region: &str,
        access_key: &Credential,
        secret_key: &Credential,
        rfc9110_date: bool,
    ) -> Result<Request<&'b [u8]>> {
        let mut request = self.request?.map(|()| body);
        self.signer.sign(&mut request, ts, region, access_key, secret_key, rfc9110_date)?;
        Ok(request)
    }
}

#[cfg(test)]
mod tests {
    use ureq::http::Method;
    use ureq::http::header;

    use crate::credential::Credential;
    use crate::s3::request_builder::RequestBuilder;
    use crate::s3::signer::Signer;

    #[test]
    fn sign() {
        let mut signer = Signer::default();
        let ts = "20130524T000000Z".parse().unwrap();
        let access_key = Credential("AKIAIOSFODNN7EXAMPLE".to_owned());
        let secret_key = Credential("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY".to_owned());
        let request = RequestBuilder::new(&mut signer)
            .method(Method::PUT)
            .uri("/test$file.text")
            .header(header::DATE, "Fri, 24 May 2013 00:00:00 GMT")
            .header(header::HOST, "examplebucket.s3.amazonaws.com")
            .header(
                "x-amz-content-sha256",
                "44ce7dd67c959e0d3524ffac1771dfbba87d2b6b4b4e99e42034a8b803f8b072",
            )
            .header("x-amz-date", "20130524T000000Z")
            .header("x-amz-storage-class", "REDUCED_REDUNDANCY")
            .sign(b"Welcome to Amazon S3.", ts, "us-east-1", &access_key, &secret_key, false)
            .unwrap();
        assert_eq!(
            request.headers()[header::AUTHORIZATION],
            include_str!("Authorization.txt").trim(),
        );
    }
}
