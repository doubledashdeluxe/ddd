// clang-format off
//
// Based on https://github.com/blckngm/noise-rust/blob/master/noise-protocol/src/traits.rs
//
// clang-format on

#include "HKDFState.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <assert.h>
#include <string.h>
}

HKDFState::HKDFState(const Array<u8, 64> &ck, const u8 *input, size_t inputSize) : m_i(1) {
    m_hmacState.init(ck);
    m_hmacState.update(input, inputSize);
    m_hmacState.digest(m_k);
    m_hmacState.init(m_k);
}

HKDFState::~HKDFState() {
    crypto_wipe(m_k.values(), m_k.count());
}

void HKDFState::update(u8 *output, size_t outputSize) {
    m_hmacState.update(&m_i, 1);
    Array<u8, 64> k;
    m_hmacState.digest(k);
    assert(outputSize <= k.count());
    memcpy(output, k.values(), outputSize);
    m_hmacState.init(m_k);
    m_hmacState.update(k.values(), k.count());
    m_i++;
}
