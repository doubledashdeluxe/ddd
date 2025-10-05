#pragma once

#include "native/online/ServerConnectionState.hh"

#include <portable/online/CookieSize.hh>

#include <array>

class ServerConnectionStateIdle : public ServerConnectionState {
public:
    ServerConnectionStateIdle(const ServerPlatform &platform);
    ~ServerConnectionStateIdle() override;
    ServerConnectionState &transfer(const u8 *input, size_t inputSize,
            std::vector<u8> &output) override;

private:
    std::array<u8, CookieSize> m_cookie;
};
