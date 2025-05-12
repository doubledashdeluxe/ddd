#pragma once

#include "native/online/ServerConnectionState.hh"
#include "native/online/ServerPlatform.hh"

#include <memory>
#include <vector>

class ServerConnection {
public:
    ServerConnection(const ServerPlatform &platform);
    ~ServerConnection();
    void transfer(const u8 *input, size_t inputSize, std::vector<u8> &output);

private:
    bool updateState(ServerConnectionState &nextState);

    std::unique_ptr<ServerConnectionState> m_state;
};
