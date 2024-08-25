// clang-format off
//
// Based on https://github.com/blckngm/noise-rust/blob/master/noise-protocol/src/symmetricstate.rs
//
// clang-format on

#include "SymmetricState.hh"

#include "payload/crypto/HKDFState.hh"

#include <common/Bytes.hh>
extern "C" {
#include <monocypher/monocypher.h>

#include <stdio.h>
#include <string.h>
}

SymmetricState::SymmetricState() {
    crypto_wipe(m_k.values(), m_k.count());
    m_nonce = UINT64_MAX;
    crypto_wipe(m_h.values(), m_h.count());
    snprintf(reinterpret_cast<char *>(m_h.values()), m_h.count(),
            "Noise_IK_25519_ChaChaPoly_BLAKE2b");
    m_ck = m_h;
}

SymmetricState::~SymmetricState() {
    crypto_wipe(m_ck.values(), m_ck.count());
    crypto_wipe(m_h.values(), m_h.count());
    crypto_wipe(m_k.values(), m_k.count());
}

void SymmetricState::mixDH(const Array<u8, 32> &k, const Array<u8, 32> &pk) {
    Array<u8, 32> sk;
    crypto_x25519(sk.values(), k.values(), pk.values());
    HKDFState hkdfState(m_ck, sk.values(), sk.count());
    crypto_wipe(sk.values(), sk.count());
    hkdfState.update(m_ck.values(), m_ck.count());
    hkdfState.update(m_k.values(), m_k.count());
    m_nonce = 0;
}

void SymmetricState::mixHash(const u8 *input, size_t inputSize) {
    crypto_blake2b_ctx ctx;
    crypto_blake2b_init(&ctx, m_h.count());
    crypto_blake2b_update(&ctx, m_h.values(), m_h.count());
    crypto_blake2b_update(&ctx, input, inputSize);
    crypto_blake2b_final(&ctx, m_h.values());
}

void SymmetricState::encryptAndHash(const u8 *input, size_t inputSize, u8 *output) {
    crypto_aead_ctx ctx;
    ctx.counter = 0;
    memcpy(ctx.key, m_k.values(), m_k.count());
    Bytes::WriteLE<u64>(ctx.nonce, 0, m_nonce);
    u8 *mac = output + inputSize;
    crypto_aead_write(&ctx, output, mac, m_h.values(), m_h.count(), input, inputSize);
    crypto_wipe(&ctx, sizeof(ctx));
    m_nonce++;
    mixHash(output, inputSize + MACSize);
}

bool SymmetricState::decryptAndHash(const u8 *input, u8 *output, size_t outputSize) {
    crypto_aead_ctx ctx;
    ctx.counter = 0;
    memcpy(ctx.key, m_k.values(), m_k.count());
    Bytes::WriteLE<u64>(ctx.nonce, 0, m_nonce);
    const u8 *mac = input + outputSize;
    if (crypto_aead_read(&ctx, output, mac, m_h.values(), m_h.count(), input, outputSize)) {
        crypto_wipe(&ctx, sizeof(ctx));
        return false;
    }
    crypto_wipe(&ctx, sizeof(ctx));
    m_nonce++;
    mixHash(input, outputSize + MACSize);
    return true;
}

void SymmetricState::computeSessionKeys(Array<u8, 32> &upstreamK, Array<u8, 32> &downstreamK) {
    HKDFState hkdfState(m_ck, nullptr, 0);
    hkdfState.update(upstreamK.values(), upstreamK.count());
    hkdfState.update(downstreamK.values(), downstreamK.count());
}
