#pragma once

#include "payload/network/CubeUDPSocket.hh"

#include <portable/network/DNS.hh>

class CubeDNS : public DNS {
public:
    static void Init();
    static CubeDNS *Instance();

private:
    CubeDNS();

    s64 secondsToTicks(s64 seconds) override;
    s64 getMonotonicTicks() override;

    CubeUDPSocket m_socket;

    static CubeDNS *s_instance;
};
