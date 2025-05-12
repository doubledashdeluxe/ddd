#pragma once

#include "native/network/FakeUDPSocket.hh"
#include "native/online/ServerConnection.hh"
#include "native/online/ServerPlatform.hh"

#include <vector>

class OnlineFakeUDPSocket final : public FakeUDPSocket {
public:
    OnlineFakeUDPSocket(const ServerPlatform &platform);
    ~OnlineFakeUDPSocket();

    s32 sendTo(const void *buffer, u32 size, const Address &address) override;

private:
    std::vector<u8> m_data;
    ServerConnection m_connection;
};
