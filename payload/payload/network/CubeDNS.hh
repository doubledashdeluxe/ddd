#pragma once

#include "payload/network/CubeUDPSocket.hh"

#include <portable/network/DNS.hh>

class CubeDNS : public DNS {
public:
    static void Init();
    static CubeDNS *Instance();

private:
    CubeDNS();

    bool ok() override;
    s32 open() override;
    s32 recvFrom(void *buffer, u32 size, Address &address) override;
    s32 sendTo(const void *buffer, u32 size, const Address &address) override;
    s64 secondsToTicks(s64 seconds) override;
    s64 getMonotonicTicks() override;

    CubeUDPSocket m_socket;

    static CubeDNS *s_instance;
};
