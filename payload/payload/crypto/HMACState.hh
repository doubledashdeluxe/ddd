#pragma once

#include <common/Array.hh>
extern "C" {
#include <monocypher/monocypher.h>
}

class HMACState {
public:
    HMACState();
    ~HMACState();
    void init(const Array<u8, 64> &k);
    void update(const u8 *input, size_t inputSize);
    void digest(Array<u8, 64> &output);

private:
    Array<u8, 128> m_opad;
    crypto_blake2b_ctx m_ctx;
};
