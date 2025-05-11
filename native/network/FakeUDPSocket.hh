#pragma once

#include <portable/network/UDPSocket.hh>

#include <optional>
#include <vector>

class FakeUDPSocket : public UDPSocket {
public:
    FakeUDPSocket(std::vector<u8> &data);
    ~FakeUDPSocket();

    s32 open() override;
    s32 close() override;
    bool ok() override;
    s32 recvFrom(void *buffer, u32 size, Address &address) override;
    s32 sendTo(const void *buffer, u32 size, const Address &address) override;

private:
    std::vector<u8> &m_data;
    std::optional<Address> m_address;
};
