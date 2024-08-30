#include "Session.hh"

#include <common/Bytes.hh>
extern "C" {
#include <monocypher/monocypher.h>
}

Session::Session() {
    reset();
}

Session::~Session() {
    crypto_wipe(m_writeK.values(), m_writeK.count());
    crypto_wipe(m_readK.values(), m_readK.count());
}

void Session::reset() {
    crypto_wipe(m_readK.values(), m_readK.count());
    crypto_wipe(m_writeK.values(), m_writeK.count());
    m_readNonce = 0;
    m_writeNonce = 0;
}

bool Session::read(u8 *buffer, u32 size, const u8 mac[MACSize], const u8 nonce[NonceSize]) {
    u64 readNonce = Bytes::ReadLE<u64>(nonce, 0);
    if (readNonce < m_readNonce || readNonce == UINT64_MAX) {
        return false;
    }
    crypto_aead_ctx ctx;
    crypto_aead_init_djb(&ctx, m_readK.values(), nonce);
    if (crypto_aead_read(&ctx, buffer, mac, nullptr, 0, buffer, size)) {
        crypto_wipe(&ctx, sizeof(ctx));
        return false;
    }
    crypto_wipe(&ctx, sizeof(ctx));
    m_readNonce = readNonce + 1;
    return true;
}

void Session::write(u8 *buffer, u32 size, u8 mac[MACSize], u8 nonce[NonceSize]) {
    Bytes::WriteLE<u64>(nonce, 0, m_writeNonce);
    crypto_aead_ctx ctx;
    crypto_aead_init_djb(&ctx, m_writeK.values(), nonce);
    crypto_aead_write(&ctx, buffer, mac, nullptr, 0, buffer, size);
    crypto_wipe(&ctx, sizeof(ctx));
    m_writeNonce++;
}
