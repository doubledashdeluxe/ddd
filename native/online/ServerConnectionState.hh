#pragma once

#include "native/online/ServerPlatform.hh"

#include <vector>

class ServerConnectionState {
public:
    ServerConnectionState(const ServerPlatform &platform);
    virtual ~ServerConnectionState();
    virtual ServerConnectionState &transfer(const u8 *input, size_t inputSize,
            std::vector<u8> &output) = 0;

protected:
    const ServerPlatform &m_platform;
};
