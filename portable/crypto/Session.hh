#pragma once

#include "portable/crypto/Types.hh"

class Session {
public:
    enum {
        MACSize = 16,
        NonceSize = 8,
    };

    Session();
    ~Session();
    void reset();
    bool read(u8 *buffer, u32 size, const u8 mac[MACSize], const u8 nonce[NonceSize]);
    void write(u8 *buffer, u32 size, u8 mac[MACSize], u8 nonce[NonceSize]);

public:
    SessionKey m_readK;
    SessionKey m_writeK;

private:
    u64 m_readNonce;
    u64 m_writeNonce;
};
