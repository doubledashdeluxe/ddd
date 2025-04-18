#pragma once

#include "portable/Array.hh"
#include "portable/crypto/HMACState.hh"

class HKDFState {
public:
    HKDFState(const Array<u8, 64> &ck, const u8 *input, size_t inputSize);
    ~HKDFState();
    void update(u8 *output, size_t outputSize);

private:
    HMACState m_hmacState;
    Array<u8, 64> m_k;
    u8 m_i;
};
