#include "Session.hh"

extern "C" {
#include <monocypher/monocypher.h>
}

Session::Session() {
    crypto_wipe(m_readK.values(), m_readK.count());
    crypto_wipe(m_writeK.values(), m_writeK.count());
}

Session::~Session() {
    crypto_wipe(m_writeK.values(), m_writeK.count());
    crypto_wipe(m_readK.values(), m_readK.count());
}
