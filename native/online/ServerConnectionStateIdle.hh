#pragma once

#include "native/online/ServerConnectionState.hh"

class ServerConnectionStateIdle : public ServerConnectionState {
public:
    ServerConnectionStateIdle(const ServerPlatform &platform);
    ~ServerConnectionStateIdle() override;
    ServerConnectionState &transfer(const u8 *input, size_t inputSize,
            std::vector<u8> &output) override;
};
