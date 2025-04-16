#pragma once

#include <portable/Array.hh>

class SymmetricState {
public:
    enum {
        MACSize = 16,
    };

    SymmetricState();
    ~SymmetricState();
    void mixDH(const Array<u8, 32> &k, const Array<u8, 32> &pk);
    void mixHash(const u8 *input, size_t inputSize);
    void encryptAndHash(const u8 *input, size_t inputSize, u8 *output);
    bool decryptAndHash(const u8 *input, u8 *output, size_t outputSize);
    void computeSessionKeys(Array<u8, 32> &upstreamK, Array<u8, 32> &downstreamK);

private:
    Array<u8, 32> m_k;
    u64 m_nonce;
    Array<u8, 64> m_h;
    Array<u8, 64> m_ck;
};
