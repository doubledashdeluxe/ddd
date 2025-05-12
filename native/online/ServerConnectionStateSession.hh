#pragma once

#include "native/online/ServerConnectionState.hh"

#include <portable/crypto/Session.hh>

class ServerConnectionStateSession : public ServerConnectionState {
public:
    ServerConnectionStateSession(const ServerPlatform &platform, Session session);
    ~ServerConnectionStateSession() override;
    ServerConnectionState &transfer(const u8 *input, size_t inputSize,
            std::vector<u8> &output) override;

private:
    Session m_session;
};
