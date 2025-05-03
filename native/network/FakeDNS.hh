#pragma once

#include <portable/network/DNS.hh>

#include <optional>
#include <vector>

class FakeDNS : public DNS {
public:
    FakeDNS(std::vector<u8> &data);
    ~FakeDNS();

private:
    bool ok() override;
    s32 open() override;
    s32 recvFrom(void *buffer, u32 size, Address &address) override;
    s32 sendTo(const void *buffer, u32 size, const Address &address) override;
    s64 secondsToTicks(s64 seconds) override;
    s64 getMonotonicTicks() override;

    std::vector<u8> &m_data;
    std::optional<Address> m_address;
    s64 m_ticks;
};
