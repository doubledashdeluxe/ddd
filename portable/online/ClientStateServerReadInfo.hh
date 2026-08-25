#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"
#include "portable/network/Address.hh"
#include "portable/online/ServerManager.hh"

#include <formats/Online.hh>

struct ClientStateServerReadInfo {
    struct Server {
        Optional<Address> address;
        Optional<u8> updateVersion;
        Optional<u16> protocolVersion;
        Optional<Array<char, MaxVersionLength + 1>> version;
        Optional<u16> courseCount;
        Optional<Array<char, MaxMotdLength + 1>> motd;
        Optional<u16> playerCount;
        bool versionIsCompatible;
        bool updateIsAvailable;
    };

    bool networkIsRunning;
    const char *networkName;
    u32 networkAddress;
    Ring<Server, MaxServerCount> servers;
};
