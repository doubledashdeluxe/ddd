#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"
#include "portable/network/Address.hh"
#include "portable/online/ServerManager.hh"

#include <formats/Online.hh>

struct ClientStateServerReadInfo {
    struct Server {
        Optional<Address> address;
        Optional<u32> protocolVersion;
        Optional<Array<char, 20>> version;
        Optional<Array<char, MaxMotdLength + 1>> motd;
        Optional<u16> playerCount;
        bool versionIsCompatible;
    };

    bool networkIsRunning;
    const char *networkName;
    u32 networkAddress;
    Ring<Server, MaxServerCount> servers;
};
