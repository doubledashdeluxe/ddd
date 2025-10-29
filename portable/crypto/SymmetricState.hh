#pragma once

#include "portable/Array.hh"
#include "portable/crypto/Types.hh"

class SymmetricState {
public:
    enum {
        MACSize = 16,
    };

    SymmetricState();
    ~SymmetricState();
    void mixDH(const Key &k, const PublicKey &pk);
    void mixHash(const u8 *input, size_t inputSize);
    void encryptAndHash(const u8 *input, size_t inputSize, u8 *output);
    bool decryptAndHash(const u8 *input, u8 *output, size_t outputSize);
    void computeSessionKeys(SessionKey &upstreamK, SessionKey &downstreamK);

private:
    Array<u8, 32> m_k;
    u64 m_nonce;
    Array<u8, 64> m_h;
    Array<u8, 64> m_ck;
};
