#pragma once

#include "payload/network/UDPSocket.hh"

#include <portable/network/DNS.hh>

class CubeDNS : public DNS {
public:
    static void Init();
    static CubeDNS *Instance();

private:
    CubeDNS();

    bool ok() override;
    s32 open() override;
    s32 recvFrom(void *buffer, u32 size, u32 &address) override;
    s32 sendTo(const void *buffer, u32 size, u32 address) override;
    s64 secondsToTicks(s64 seconds) override;
    s64 getMonotonicTicks() override;

    UDPSocket m_socket;

    static CubeDNS *s_instance;
};
