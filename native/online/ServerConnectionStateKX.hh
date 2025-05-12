#pragma once

#include "native/online/ServerConnectionState.hh"

#include <portable/crypto/KX.hh>
#include <portable/crypto/Session.hh>

#include <array>

class ServerConnectionStateKX : public ServerConnectionState {
public:
    ServerConnectionStateKX(const ServerPlatform &platform, const std::array<u8, KX::M2Size> &m2,
            const Session &session);
    ~ServerConnectionStateKX() override;
    ServerConnectionState &transfer(const u8 *input, size_t inputSize,
            std::vector<u8> &output) override;

private:
    std::array<u8, KX::M2Size> m_m2;
    Session m_session;
};
