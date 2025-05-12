#pragma once

#include <portable/network/DNS.hh>

class FakeDNS final : public DNS {
public:
    FakeDNS(UDPSocket &socket);
    ~FakeDNS();

private:
    s64 secondsToTicks(s64 seconds) override;
    s64 getMonotonicTicks() override;

    s64 m_ticks;
};
