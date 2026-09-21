use std::sync::Arc;

use ureq::Agent;
use ureq::tls::{TlsConfig, TlsProvider};

pub fn agent() -> Agent {
    let crypto_provider = Arc::new(rustls_graviola::default_provider());
    Agent::config_builder()
        .tls_config(
            TlsConfig::builder()
                .provider(TlsProvider::Rustls)
                .unversioned_rustls_crypto_provider(crypto_provider)
                .build(),
        )
        .build()
        .new_agent()
}
