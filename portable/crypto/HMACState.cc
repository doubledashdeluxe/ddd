// clang-format off
//
// Based on https://github.com/blckngm/noise-rust/blob/master/noise-protocol/src/traits.rs
//
// clang-format on

#include "HMACState.hh"

HMACState::HMACState() {}

HMACState::~HMACState() {
    crypto_wipe(&m_ctx, sizeof(m_ctx));
    crypto_wipe(m_opad.values(), m_opad.count());
}

void HMACState::init(const Array<u8, 64> &k) {
    Array<u8, 128> ipad(0x36);
    m_opad.fill(0x5c);
    for (size_t i = 0; i < k.count(); i++) {
        ipad[i] ^= k[i];
        m_opad[i] ^= k[i];
    }
    crypto_blake2b_init(&m_ctx, 64);
    crypto_blake2b_update(&m_ctx, ipad.values(), ipad.count());
}

void HMACState::update(const u8 *input, size_t inputSize) {
    crypto_blake2b_update(&m_ctx, input, inputSize);
}

void HMACState::digest(Array<u8, 64> &output) {
    crypto_blake2b_final(&m_ctx, output.values());

    crypto_blake2b_init(&m_ctx, output.count());
    crypto_blake2b_update(&m_ctx, m_opad.values(), m_opad.count());
    crypto_blake2b_update(&m_ctx, output.values(), output.count());
    crypto_blake2b_final(&m_ctx, output.values());
    crypto_wipe(m_opad.values(), m_opad.count());
}
