#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"
#include "portable/Version.hh"
#include "portable/network/Address.hh"
#include "portable/online/ServerManager.hh"

struct ClientStateServerInfo {
    struct Server {
        Optional<Address> address;
        Optional<u32> protocolVersion;
        Optional<Version> version;
        Optional<Array<char, 100>> motd;
        Optional<u16> playerCount;
        bool versionIsCompatible;
    };

    bool networkIsRunning;
    const char *networkName;
    u32 networkAddress;
    Ring<Server, ServerManager::MaxServerCount> servers;
};
